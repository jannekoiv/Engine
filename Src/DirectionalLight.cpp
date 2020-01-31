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

static Material createMaterial(
    Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain, Texture depthTexture)
{
    auto vertexShader = createShaderFromFile(device, "d:/Shaders/shadowvert.spv");
    depthTexture.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::vector<Texture> textures{};
    return Material{
        device, descriptorManager, swapChain, &depthTexture, textures, vertexShader, nullptr, MaterialUsage::ShadowMap};
}

static DescriptorSet createDescriptorSet(DescriptorManager& descriptorManager, vk::Buffer uniformBuffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(LightUniform);

    descriptorSet.writeDescriptors({{0, 0, 1, &bufferInfo}});
    return descriptorSet;
}

static Buffer createUniformBuffer(Device& device)
{
    return Buffer(
        device,
        sizeof(LightUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

DirectionalLight::DirectionalLight(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain)
    : mDevice{device},
      mSwapChain{swapChain},
      mDepthTexture{
          device,
          vk::ImageViewType::e2D,
          1,
          vk::Extent3D{swapChain.extent().width, swapChain.extent().height, 1},
          findDepthAttachmentFormat(device),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eDepthStencilAttachment,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge},
      mMaterial{createMaterial(mDevice, descriptorManager, swapChain, mDepthTexture)},
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
    std::cout << "Light constructed\n";
}

void DirectionalLight::createCommandBuffers(std::vector<Model>& models, vk::Extent2D swapChainExtent)
{
    vk::CommandBufferAllocateInfo commandBufferInfo{mDevice.commandPool(), vk::CommandBufferLevel::ePrimary, 1};

    mCommandBuffer = static_cast<vk::Device>(mDevice).allocateCommandBuffers(commandBufferInfo).front();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    beginInfo.pInheritanceInfo = nullptr;
    mCommandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = mMaterial.framebufferSet().renderPass();
    renderPassInfo.framebuffer = mMaterial.framebufferSet().frameBuffer(0);
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<vk::ClearValue, 1> clearValues;
    clearValues[0].depthStencil = vk::ClearDepthStencilValue{1.0f, 150};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    mCommandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
    for (Model& model : models) {
        mCommandBuffer.bindVertexBuffers(0, {model.vertexBuffer()}, {0});
        mCommandBuffer.bindIndexBuffer(model.indexBuffer(), 0, vk::IndexType::eUint32);

        mCommandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, mPipeline.layout(), 0, {mDescriptorSet}, nullptr);

        mCommandBuffer.drawIndexed(static_cast<uint32_t>(model.indexCount()), 1, 0, 0, 0);
    }

    mCommandBuffer.endRenderPass();
    mCommandBuffer.end();
}

void DirectionalLight::drawFrame()
{
    updateUniformBuffer();
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &mCommandBuffer, 0, nullptr);
    mDevice.graphicsQueue().submit(submitInfo, nullptr);
    mDevice.graphicsQueue().waitIdle();
}

void DirectionalLight::updateUniformBuffer()
{
    void* data = static_cast<vk::Device>(mDevice).mapMemory(mUniformBuffer.memory(), 0, sizeof(mUniform), {});
    memcpy(data, &mUniform, sizeof(LightUniform));
    static_cast<vk::Device>(mDevice).unmapMemory(mUniformBuffer.memory());
}
