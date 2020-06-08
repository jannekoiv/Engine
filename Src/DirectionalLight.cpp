#include "../Include/DirectionalLight.h"
#include "../Include/Device.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Quad.h"
#include "../Include/Skybox.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"
#include <fstream>
#include <iostream>
#include <stb_image.h>

static glm::mat4 orthoProjMatrix()
{
    const float sizeX = 20.0f;
    const float sizeY = 9.f / 16.f * sizeX;
    return glm::ortho(-sizeX, sizeX, sizeY, -sizeY, 1.f, 50.f);
}

static vk::CommandBuffer createCommandBuffer(Device& device)
{
    vk::CommandBufferAllocateInfo commandBufferInfo{
        device.commandPool(), vk::CommandBufferLevel::ePrimary, 1};
    auto commandBuffer =
        static_cast<vk::Device>(device).allocateCommandBuffers(commandBufferInfo).front();
    return commandBuffer;
}

DirectionalLight::DirectionalLight(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain)
    : mDevice{device},
      mSwapChain{swapChain},
      mCommandBuffer{createCommandBuffer(mDevice)},
      mWorldMatrix{},
      mProjMatrix{orthoProjMatrix()},
      mDepthTexture{
          device,
          vk::ImageViewType::e2D,
          1,
          vk::Extent3D{swapChain.extent().width, swapChain.extent().height, 1},
          findDepthAttachmentFormat(device),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eDepthStencilAttachment,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToBorder},
      mPipeline{
          mDevice,
          descriptorManager,
          textureManager,
          swapChain,
          &mDepthTexture,
          Vertex::bindingDescription(),
          Vertex::attributeDescriptions(),
          nullptr,
          {{"vertexShader", "d:/Shaders/shadowvert.spv"}, {"usage", "ShadowMap"}}}
{
    mDepthTexture.transitionLayout(
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::cout << "Directional light constructed.\n";
}

void DirectionalLight::drawFrame(std::vector<Object>& objects, vk::Extent2D swapChainExtent)
{
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    mCommandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = mPipeline.framebufferSet().renderPass();
    renderPassInfo.framebuffer = mPipeline.framebufferSet().frameBuffer(0);
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<vk::ClearValue, 1> clearValues;
    clearValues[0].depthStencil = vk::ClearDepthStencilValue{1.0f, 150};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    mCommandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);

    for (auto& object : objects) {
        mCommandBuffer.bindVertexBuffers(0, {object.vertexBuffer()}, {0});
        mCommandBuffer.bindIndexBuffer(object.indexBuffer(), 0, vk::IndexType::eUint32);

        glm::mat4 worldViewProj = mProjMatrix * viewMatrix() * object.worldMatrix();
        mCommandBuffer.pushConstants(
            mPipeline.layout(),
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(float) * 16,
            &worldViewProj);

        mCommandBuffer.drawIndexed(static_cast<uint32_t>(object.indexCount()), 1, 0, 0, 0);
    }

    mCommandBuffer.endRenderPass();
    mCommandBuffer.end();

    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &mCommandBuffer, 0, nullptr);
    mDevice.graphicsQueue().submit(submitInfo, nullptr);
    mDevice.graphicsQueue().waitIdle();
    mCommandBuffer.reset({});
}
