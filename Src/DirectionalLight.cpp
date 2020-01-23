#include "../Include/DirectionalLight.h"
#include "../Include/Device.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Model.h"
#include "../Include/Quad.h"
#include "../Include/Skybox.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"
#include "stb_image.h"
#include <fstream>
#include <iostream>

static Material createMaterial(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain)
{
    auto vertexShader = createShaderFromFile(device, "d:/Shaders/shadowvert.spv");
    auto fragmentShader = createShaderFromFile(device, "d:/Shaders/shadowfrag.spv");

    Texture texture{
        device,
        vk::ImageViewType::e2D,
        1,
        vk::Extent3D(swapChain.extent().width, swapChain.extent().height, 1),
        vk::Format::eB8G8R8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eColorAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eRepeat};

    texture.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    return Material{
        device, descriptorManager, swapChain, nullptr, texture, vertexShader, fragmentShader, MaterialUsage::ShadowMap};
}

static DescriptorSet createDescriptorSet(DescriptorManager& descriptorManager, vk::Buffer uniformBuffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(ModelUniform);

    descriptorSet.writeDescriptors({{0, 0, 1, &bufferInfo}});
    return descriptorSet;
}

static Buffer createUniformBuffer(Device& device)
{
    return Buffer(
        device,
        sizeof(ModelUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

DirectionalLight::DirectionalLight(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain)
    : mDevice{device},
      mSwapChain{swapChain},
      mImageAvailableSemaphore{static_cast<vk::Device>(mDevice).createSemaphore({})},
      mRenderFinishedSemaphore{static_cast<vk::Device>(mDevice).createSemaphore({})},
      mMaterial{createMaterial(mDevice, descriptorManager, swapChain)},
      mUniformBuffer{createUniformBuffer(mDevice)},
      mDescriptorSet{createDescriptorSet(descriptorManager, mUniformBuffer)},
      mPipeline{
          mDevice,
          mMaterial,
          mDescriptorSet.layout(),
          ModelVertex::bindingDescription(),
          ModelVertex::attributeDescriptions(),
          swapChain.extent(),
          mMaterial.materialUsage()}
{
    std::cout << "Directional light initialized\n";
}

void DirectionalLight::createCommandBuffers(std::vector<Model>& models, vk::Extent2D swapChainExtent)
{
    vk::CommandBufferAllocateInfo commandBufferInfo{mDevice.commandPool(), vk::CommandBufferLevel::ePrimary, 1};

    mCommandBuffer = static_cast<vk::Device>(mDevice).allocateCommandBuffers(commandBufferInfo).front();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    beginInfo.pInheritanceInfo = nullptr;
    mCommandBuffer.begin(beginInfo);

    for (Model& model : models) {
        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = mMaterial.framebufferSet().renderPass();
        renderPassInfo.framebuffer = mMaterial.framebufferSet().frameBuffer(0);
        renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = std::array<float, 4>{0.2f, 0.4f, 0.5f, 1.0f};
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        mCommandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
        mCommandBuffer.bindVertexBuffers(0, {model.vertexBuffer()}, {0});
        mCommandBuffer.bindIndexBuffer(model.indexBuffer(), 0, vk::IndexType::eUint32);

        mCommandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, mPipeline.layout(), 0, {mDescriptorSet}, nullptr);

        //mCommandBuffer.bindDescriptorSets(
        //    vk::PipelineBindPoint::eGraphics,
        //    mPipeline.layout(),
        //    1,
        //    {mMaterial.descriptorSet()},
        //    nullptr);

        mCommandBuffer.drawIndexed(static_cast<uint32_t>(model.indexCount()), 1, 0, 0, 0);

        mCommandBuffer.endRenderPass();
    }

    mCommandBuffer.end();
}

void DirectionalLight::drawFrame()
{
    updateUniformBuffer();
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &mCommandBuffer, 0, nullptr);
    mDevice.graphicsQueue().submit(submitInfo, nullptr);
    mDevice.graphicsQueue().waitIdle();

    //uint32_t imageIndex = 0;
    //static_cast<vk::Device>(mDevice).acquireNextImageKHR(
    //    mSwapChain, std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphore, nullptr, &imageIndex);

    //vk::SubmitInfo submitInfo;
    //vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    //submitInfo.waitSemaphoreCount = 1;
    //submitInfo.pWaitSemaphores = &mImageAvailableSemaphore;
    //submitInfo.pWaitDstStageMask = waitStages;

    //submitInfo.commandBufferCount = 1;
    //submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

    //submitInfo.signalSemaphoreCount = 1;
    //submitInfo.pSignalSemaphores = &mRenderFinishedSemaphore;

    //mDevice.graphicsQueue().submit(submitInfo, nullptr);

    //vk::PresentInfoKHR presentInfo;
    //presentInfo.waitSemaphoreCount = 1;
    //presentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;

    //presentInfo.swapchainCount = 1;
    //presentInfo.pSwapchains = &static_cast<vk::SwapchainKHR>(mSwapChain);
    //presentInfo.pImageIndices = &imageIndex;
    //presentInfo.pResults = nullptr;

    //mDevice.presentQueue().presentKHR(presentInfo);
    //static_cast<vk::Device>(mDevice).waitIdle();
}

void DirectionalLight::updateUniformBuffer()
{
    void* data = static_cast<vk::Device>(mDevice).mapMemory(mUniformBuffer.memory(), 0, sizeof(mUniform), {});
    memcpy(data, &mUniform, sizeof(ModelUniform));
    static_cast<vk::Device>(mDevice).unmapMemory(mUniformBuffer.memory());
}
