
#include "../Include/Renderer.h"
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

std::vector<vk::CommandBuffer> createCommandBuffers(Engine& engine, std::vector<Model*> models);
UboWorldView createUniformObject(Engine& engine, std::vector<Model*> models);
Buffer createUniformBuffer(Engine& engine, std::vector<Model*> models);

Renderer::Renderer(Engine& engine, std::vector<Model*> models)
    : mEngine(engine),
      mCommandBuffers(createCommandBuffers(engine, models)),
      mUboWorldView(createUniformObject(mEngine, models)),
      mUniformBuffer(createUniformBuffer(mEngine, models)),
      mImageAvailableSemaphore(static_cast<vk::Device>(mEngine.device()).createSemaphore({})),
      mRenderFinishedSemaphore(static_cast<vk::Device>(mEngine.device()).createSemaphore({}))
{
}

int calcUboAlignment(Engine& engine)
{
    auto minUboAlignment =
        engine.device().physicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
    auto uboAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        uboAlignment = (uboAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return uboAlignment;
}

UboWorldView createUniformObject(Engine& engine, std::vector<Model*> models)
{
    UboWorldView ubo;
    auto uboAlignment = calcUboAlignment(engine);
    ubo.worldView = (glm::mat4*)alignedAlloc(uboAlignment * models.size(), uboAlignment);
    return ubo;
}

Buffer createUniformBuffer(Engine& engine, std::vector<Model*> models)
{
    auto bufferSize = calcUboAlignment(engine) * models.size();

    Buffer buffer(
        engine.device(),
        bufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostCoherent);

    return buffer;
}

std::vector<vk::CommandBuffer> createCommandBuffers(Engine& engine, std::vector<Model*> models)
{
    vk::CommandBufferAllocateInfo commandBufferInfo(
        engine.device().commandPool(), vk::CommandBufferLevel::ePrimary, engine.frameBufferCount());

    auto commandBuffers =
        static_cast<vk::Device>(engine.device()).allocateCommandBuffers(commandBufferInfo);

    size_t frameBufferIndex = 0;
    for (vk::CommandBuffer commandBuffer : commandBuffers) {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
        beginInfo.pInheritanceInfo = nullptr;
        commandBuffer.begin(beginInfo);

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = engine.renderPass();
        renderPassInfo.framebuffer = engine.frameBuffer(frameBufferIndex++);
        renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderPassInfo.renderArea.extent = engine.swapChain().extent();

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

void Renderer::drawFrame()
{
    uint32_t imageIndex = 0;
    static_cast<vk::Device>(mEngine.device())
        .acquireNextImageKHR(
            mEngine.swapChain(),
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

    mEngine.device().graphicsQueue().submit(submitInfo, nullptr);

    vk::PresentInfoKHR presentInfo;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;

    vk::SwapchainKHR swapChains[] = {mEngine.swapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    mEngine.device().presentQueue().presentKHR(presentInfo);
    static_cast<vk::Device>(mEngine.device()).waitIdle();
}
