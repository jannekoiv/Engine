
#include "../Include/Renderer.h"
#include "../Include/Device.h"
#include "../Include/Framebuffer.h"
#include "../Include/Image.h"
#include "../Include/Model.h"
#include "../Include/RenderPass.h"
#include "../Include/SwapChain.h"
#include <fstream>
#include <iostream>

void* alignedAlloc(size_t size, size_t alignment)
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

void alignedFree(void* data)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(data);
#else
    free(data);
#endif
}

//static std::vector<Framebuffer> createFramebuffers(
//    Device& device, SwapChain& swapChain, Image& depthImage, RenderPass& renderPass)
//{
//    std::vector<Framebuffer> frameBuffers;
//    for (int i = 0; i < swapChain.imageCount(); i++) {
//        frameBuffers.emplace_back(
//            device, swapChain.imageView(i), depthImage.view(), swapChain.extent(), renderPass);
//    }
//    return frameBuffers;
//}

void clearColor(Device& device, SwapChain& swapChain, int index, vk::CommandBuffer commandBuffer)
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
        swapChain.image(index), vk::ImageLayout::eTransferDstOptimal, clearColor, imageRange);

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clearToPresentBarrier});
}

void clearDepthStencil(Device& device, Image& depthImage, vk::CommandBuffer commandBuffer)
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
    presentToClearBarrier.image = depthImage;
    presentToClearBarrier.subresourceRange = imageRange;

    // Change layout of image to be optimal for presenting
    vk::ImageMemoryBarrier clearToPresentBarrier{};
    clearToPresentBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    clearToPresentBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    clearToPresentBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    clearToPresentBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.image = depthImage;
    clearToPresentBarrier.subresourceRange = imageRange;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        {},
        {},
        {presentToClearBarrier});

    commandBuffer.clearDepthStencilImage(
        depthImage, vk::ImageLayout::eTransferDstOptimal, clearDepthStencil, imageRange);

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clearToPresentBarrier});
}

void clearPass(
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

void drawModelsPass(
    vk::CommandBuffer commandBuffer,
    int framebufferIndex,
    vk::Extent2D swapChainExtent,
    std::vector<Model>& models)
{
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = models.front().mMaterial.renderPass();
    renderPassInfo.framebuffer = models.front().mMaterial.frameBuffer(framebufferIndex);
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    for (Model& model : models) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, model.mMaterial.pipeline());
        commandBuffer.bindVertexBuffers(0, {model.vertexBuffer()}, {0});
        commandBuffer.bindIndexBuffer(model.indexBuffer(), 0, vk::IndexType::eUint32);

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            model.mMaterial.pipeline().layout(),
            0,
            model.descriptorSet(),
            nullptr);

        commandBuffer.drawIndexed(static_cast<uint32_t>(model.indexCount()), 1, 0, 0, 0);
    }

    commandBuffer.endRenderPass();
}

std::vector<vk::CommandBuffer> createCommandBuffers(
    Device& device,
    SwapChain& swapChain,
    Image& depthImage,
    std::vector<Model>& models,
    RenderPass& clearRenderPass,
    std::vector<Framebuffer>& clearFrameBuffers)
{
    vk::CommandBufferAllocateInfo commandBufferInfo(
        device.commandPool(), vk::CommandBufferLevel::ePrimary, swapChain.imageCount());

    auto commandBuffers = static_cast<vk::Device>(device).allocateCommandBuffers(commandBufferInfo);

    size_t framebufferIndex = 0;
    for (vk::CommandBuffer commandBuffer : commandBuffers) {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
        beginInfo.pInheritanceInfo = nullptr;
        commandBuffer.begin(beginInfo);

        clearPass(
            commandBuffer,
            clearRenderPass,
            clearFrameBuffers[framebufferIndex],
            swapChain.extent());

        drawModelsPass(commandBuffer, framebufferIndex, swapChain.extent(), models);

        framebufferIndex++;
        commandBuffer.end();
    }
    return commandBuffers;
}

static std::vector<Framebuffer> createFramebuffers(
    Device& device, SwapChain& swapChain, Image& depthImage, RenderPass& renderPass)
{
    std::vector<Framebuffer> frameBuffers;
    for (int i = 0; i < swapChain.imageCount(); i++) {
        frameBuffers.emplace_back(
            device, swapChain.imageView(i), depthImage.view(), swapChain.extent(), renderPass);
    }
    return frameBuffers;
}

Renderer::Renderer(
    Device& device, SwapChain& swapChain, Image& depthImage, std::vector<Model>& models)
    : mDevice(device),
      mSwapChain(swapChain),
      mDepthImage(depthImage),
      mClearRenderPass{mDevice, swapChain.format(), vk::AttachmentLoadOp::eClear},
      mClearFramebuffers{createFramebuffers(mDevice, mSwapChain, mDepthImage, mClearRenderPass)},
      mCommandBuffers(createCommandBuffers(
          mDevice, mSwapChain, mDepthImage, models, mClearRenderPass, mClearFramebuffers)),
      mImageAvailableSemaphore(static_cast<vk::Device>(mDevice).createSemaphore({})),
      mRenderFinishedSemaphore(static_cast<vk::Device>(mDevice).createSemaphore({}))
{
}

//int calcUboAlignment(Engine& engine)
//{
//    auto minUboAlignment =
//        engine.device().physicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
//    auto uboAlignment = sizeof(glm::mat4);
//    if (minUboAlignment > 0) {
//        uboAlignment = (uboAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
//    }
//    return uboAlignment;
//}
//
//UboWorldView createUniformObject(Engine& engine, std::vector<Model*> models)
//{
//    UboWorldView ubo;
//    auto uboAlignment = calcUboAlignment(engine);
//    ubo.worldView = (glm::mat4*)alignedAlloc(uboAlignment * models.size(), uboAlignment);
//    return ubo;
//}
//
//Buffer createUniformBuffer(Engine& engine, std::vector<Model*> models)
//{
//    auto bufferSize = calcUboAlignment(engine) * models.size();
//
//    Buffer buffer(
//        engine.device(),
//        bufferSize,
//        vk::BufferUsageFlagBits::eUniformBuffer,
//        vk::MemoryPropertyFlagBits::eHostCoherent);
//
//    return buffer;
//}

void Renderer::drawFrame()
{
    uint32_t imageIndex = 0;
    static_cast<vk::Device>(mDevice).acquireNextImageKHR(
        mSwapChain,
        std::numeric_limits<uint64_t>::max(),
        mImageAvailableSemaphore,
        nullptr,
        &imageIndex);

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mImageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mRenderFinishedSemaphore;

    mDevice.graphicsQueue().submit(submitInfo, nullptr);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &static_cast<vk::SwapchainKHR>(mSwapChain);
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    mDevice.presentQueue().presentKHR(presentInfo);
    static_cast<vk::Device>(mDevice).waitIdle();
}
