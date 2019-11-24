
#include "../Include/Renderer.h"
#include "../Include/Device.h"
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

//UboWorldView createUniformObject(Engine& engine, std::vector<Model*> models);
//Buffer createUniformBuffer(Engine& engine, std::vector<Model*> models);

std::vector<vk::CommandBuffer> createCommandBuffers(
    Device& device,
    std::vector<Model*> models,
    std::vector<vk::Framebuffer>& frameBuffers,
    RenderPass& renderPass,
    SwapChain& swapChain)
{
    vk::CommandBufferAllocateInfo commandBufferInfo(
        device.commandPool(), vk::CommandBufferLevel::ePrimary, frameBuffers.size());

    auto commandBuffers = static_cast<vk::Device>(device).allocateCommandBuffers(commandBufferInfo);

    size_t frameBufferIndex = 0;
    for (vk::CommandBuffer commandBuffer : commandBuffers) {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
        beginInfo.pInheritanceInfo = nullptr;
        commandBuffer.begin(beginInfo);

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffers[frameBufferIndex++];
        renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderPassInfo.renderArea.extent = swapChain.extent();

        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f};
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, models.front()->mPipeline);

        for (Model* model : models) {
            std::array<vk::Buffer, 1> buffers = {static_cast<vk::Buffer>(model->vertexBuffer())};
            std::array<vk::DeviceSize, 1> sizes = {0};
            commandBuffer.bindVertexBuffers(0, buffers, sizes);

            commandBuffer.bindIndexBuffer(model->indexBuffer(), 0, vk::IndexType::eUint32);

            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                models.front()->mPipeline.layout(),
                0,
                model->descriptorSet(),
                nullptr);

            commandBuffer.drawIndexed(static_cast<uint32_t>(model->indexCount()), 1, 0, 0, 0);
        }

        commandBuffer.endRenderPass();
        commandBuffer.end();
    }
    return commandBuffers;
}

Renderer::Renderer(
    Device& device,
    std::vector<Model*> models,
    std::vector<vk::Framebuffer>& frameBuffers,
    RenderPass& renderPass,
    SwapChain& swapChain)
    : mDevice(device),
      mCommandBuffers(createCommandBuffers(device, models, frameBuffers, renderPass, swapChain)),
      //      mUboWorldView(createUniformObject(mEngine, models)),
      //      mUniformBuffer(createUniformBuffer(mEngine, models)),
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

void Renderer::drawFrame(SwapChain& swapChain)
{
    uint32_t imageIndex = 0;
    static_cast<vk::Device>(mDevice).acquireNextImageKHR(
        swapChain,
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
    presentInfo.pSwapchains = &static_cast<vk::SwapchainKHR>(swapChain);
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    mDevice.presentQueue().presentKHR(presentInfo);
    static_cast<vk::Device>(mDevice).waitIdle();
}
