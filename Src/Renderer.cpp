
#include "../Include/Renderer.h"
#include "../Include/Device.h"
#include "../Include/DirectionalLight.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Quad.h"
#include "../Include/Skybox.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"
#include <fstream>
#include <iostream>

std::vector<vk::CommandBuffer> createCommandBuffers(Device& device, SwapChain& swapChain)
{
    vk::CommandBufferAllocateInfo commandBufferInfo(
        device.commandPool(),
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(swapChain.imageCount()));

    std::vector<vk::CommandBuffer> commandBuffers =
        static_cast<vk::Device>(device).allocateCommandBuffers(commandBufferInfo);

    return commandBuffers;
}

static DescriptorSet createDescriptorSet(
    DescriptorManager& descriptorManager, vk::Buffer uniformBuffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0,
         vk::DescriptorType::eUniformBuffer,
         1,
         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(SceneUniform);

    std::vector<DescriptorWrite> writes = {
        {0, 0, 1, &bufferInfo}}; //, {1, 0, 1, &imageInfo}};
    descriptorSet.writeDescriptors(writes);
    return descriptorSet;
}

Renderer::Renderer(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthTexture)
    : mDevice{device},
      mSwapChain{swapChain},
      mDepthTexture{depthTexture},
      mClearFramebufferSet{mDevice, mSwapChain, &mDepthTexture, {{"usage", "Clear"}}},
      mCommandBuffers{createCommandBuffers(device, swapChain)},
      mImageAvailableSemaphore{static_cast<vk::Device>(mDevice).createSemaphore({})},
      mRenderFinishedSemaphore{static_cast<vk::Device>(mDevice).createSemaphore({})},
      mUniform{},
      mUniformBuffer{
          device,
          sizeof(SceneUniform),
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent},
      mDescriptorSet{createDescriptorSet(descriptorManager, mUniformBuffer)}
{
    std::cout << "Renderer initialized\n";
}

Renderer::~Renderer()
{
    static_cast<vk::Device>(mDevice).destroySemaphore(mRenderFinishedSemaphore);
    static_cast<vk::Device>(mDevice).destroySemaphore(mImageAvailableSemaphore);
}

static void* alignedAlloc(size_t size, size_t alignment)
{
    void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc(size, alignment);
#else
    int res = posix_memalign(&data, alignment, size);
    if (res != 0)
        data = nullptr;
#endif
    return data;
}

static void alignedFree(void* data)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(data);
#else
    free(data);
#endif
}

static void clearColor(
    Device& device, SwapChain& swapChain, int index, vk::CommandBuffer commandBuffer)
{
    vk::ClearColorValue clearColor{std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f}};

    vk::ImageSubresourceRange imageRange{};
    imageRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageRange.baseMipLevel = 0;
    imageRange.levelCount = 1;
    imageRange.baseArrayLayer = 0;
    imageRange.layerCount = 1;

    vk::ImageMemoryBarrier presentToClearBarrier{};
    presentToClearBarrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    presentToClearBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    presentToClearBarrier.oldLayout = vk::ImageLayout::eUndefined;
    presentToClearBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    presentToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentToClearBarrier.image = swapChain.image(index);
    presentToClearBarrier.subresourceRange = imageRange;

    // Change layout of image to be optimal for presenting
    vk::ImageMemoryBarrier clearToPresentBarrier{};
    clearToPresentBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    clearToPresentBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    clearToPresentBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    clearToPresentBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.image = swapChain.image(index);
    clearToPresentBarrier.subresourceRange = imageRange;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        {},
        {},
        {presentToClearBarrier});

    commandBuffer.clearColorImage(
        swapChain.image(index),
        vk::ImageLayout::eTransferDstOptimal,
        clearColor,
        imageRange);

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clearToPresentBarrier});
}

static void clearDepthStencil(
    Device& device, Texture& depthTexture, vk::CommandBuffer commandBuffer)
{
    vk::ClearDepthStencilValue clearDepthStencil{1.0f, 0};

    vk::ImageSubresourceRange imageRange{};
    imageRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    imageRange.baseMipLevel = 0;
    imageRange.levelCount = 1;
    imageRange.baseArrayLayer = 0;
    imageRange.layerCount = 1;

    vk::ImageMemoryBarrier presentToClearBarrier{};
    presentToClearBarrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    presentToClearBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    presentToClearBarrier.oldLayout = vk::ImageLayout::eUndefined;
    presentToClearBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    presentToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentToClearBarrier.image = depthTexture.image();
    presentToClearBarrier.subresourceRange = imageRange;

    // Change layout of image to be optimal for presenting
    vk::ImageMemoryBarrier clearToPresentBarrier{};
    clearToPresentBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    clearToPresentBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    clearToPresentBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    clearToPresentBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.image = depthTexture.image();
    clearToPresentBarrier.subresourceRange = imageRange;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        {},
        {},
        {presentToClearBarrier});

    commandBuffer.clearDepthStencilImage(
        depthTexture.image(),
        vk::ImageLayout::eTransferDstOptimal,
        clearDepthStencil,
        imageRange);

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clearToPresentBarrier});
}

static void clearPass(
    vk::CommandBuffer commandBuffer,
    vk::RenderPass renderPass,
    vk::Framebuffer frameBuffer,
    vk::Extent2D swapChainExtent)
{
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = frameBuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.endRenderPass();
}

float t2 = 1.0f;

static void drawObjectsPass(
    DescriptorSet& descriptorSet,
    vk::CommandBuffer commandBuffer,
    int framebufferIndex,
    vk::Extent2D swapChainExtent,
    std::vector<Object>& objects)
{
    for (auto& object : objects) {
        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = object.pipeline().framebufferSet().renderPass();
        renderPassInfo.framebuffer =
            object.pipeline().framebufferSet().frameBuffer(framebufferIndex);
        renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderPassInfo.renderArea.extent = swapChainExtent;

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, object.pipeline());
        commandBuffer.bindVertexBuffers(0, {object.vertexBuffer()}, {0});
        commandBuffer.bindIndexBuffer(object.indexBuffer(), 0, vk::IndexType::eUint32);

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            object.pipeline().layout(),
            0,
            {descriptorSet, object.pipeline().descriptorSet(), object.descriptorSet()},
            nullptr);

        commandBuffer.drawIndexed(static_cast<uint32_t>(object.indexCount()), 1, 0, 0, 0);

        commandBuffer.endRenderPass();
    }
}

static void drawSkyboxPass(
    vk::CommandBuffer commandBuffer,
    int framebufferIndex,
    vk::Extent2D swapChainExtent,
    Skybox& skybox)
{
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = skybox.pipeline().framebufferSet().renderPass();
    renderPassInfo.framebuffer =
        skybox.pipeline().framebufferSet().frameBuffer(framebufferIndex);
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skybox.pipeline());
    commandBuffer.bindVertexBuffers(0, {skybox.vertexBuffer()}, {0});
    commandBuffer.bindIndexBuffer(skybox.indexBuffer(), 0, vk::IndexType::eUint32);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        skybox.pipeline().layout(),
        0,
        {skybox.descriptorSet()},
        nullptr);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        skybox.pipeline().layout(),
        1,
        {skybox.pipeline().descriptorSet()},
        nullptr);

    commandBuffer.drawIndexed(36, 1, 0, 0, 0);

    commandBuffer.endRenderPass();
}

static void drawQuadPass(
    vk::CommandBuffer commandBuffer,
    int framebufferIndex,
    vk::Extent2D swapChainExtent,
    Quad& quad)
{
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = quad.pipeline().framebufferSet().renderPass();
    renderPassInfo.framebuffer =
        quad.pipeline().framebufferSet().frameBuffer(framebufferIndex);
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, quad.pipeline());
    commandBuffer.bindVertexBuffers(0, {quad.vertexBuffer()}, {0});

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        quad.pipeline().layout(),
        0,
        {quad.descriptorSet()},
        nullptr);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        quad.pipeline().layout(),
        1,
        {quad.pipeline().descriptorSet()},
        nullptr);

    commandBuffer.draw(6, 1, 0, 0);

    commandBuffer.endRenderPass();
}

void Renderer::drawFrame(std::vector<Object>& objects)
{
    uint32_t imageIndex = 0;
    static_cast<vk::Device>(mDevice).acquireNextImageKHR(
        mSwapChain,
        std::numeric_limits<uint64_t>::max(),
        mImageAvailableSemaphore,
        nullptr,
        &imageIndex);

    vk::CommandBuffer commandBuffer = mCommandBuffers[imageIndex];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);

    clearPass(
        commandBuffer,
        mClearFramebufferSet.renderPass(),
        mClearFramebufferSet.frameBuffer(imageIndex),
        mSwapChain.extent());

    drawObjectsPass(
        mDescriptorSet, commandBuffer, imageIndex, mSwapChain.extent(), objects);

    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitStages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mImageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mRenderFinishedSemaphore;

    mDevice.graphicsQueue().submit(submitInfo, nullptr);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    auto swapChain = static_cast<vk::SwapchainKHR>(mSwapChain);
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    mDevice.presentQueue().presentKHR(presentInfo);
    static_cast<vk::Device>(mDevice).waitIdle();
}

void Renderer::updateUniformBuffer(
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix,
    const glm::mat4& lightSpace,
    const glm::vec3& lightDir)
{
    mUniform.view = viewMatrix;
    mUniform.proj = projMatrix;
    mUniform.lightSpace = lightSpace;
    mUniform.lightDir = lightDir;

    void* data = mUniformBuffer.mapMemory();
    memcpy(data, &mUniform, sizeof(SceneUniform));
    mUniformBuffer.unmapMemory();
}




