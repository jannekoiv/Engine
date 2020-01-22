#include "../Include/DirectionalLight.h"
#include "../Include/Device.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Model.h"
#include "../Include/Quad.h"
#include "../Include/Skybox.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"
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
        vk::Extent3D{swapChain.extent()},
        findDepthAttachmentFormat(device),
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eClampToEdge};

    std::cout << "TRANSITION\n";
    texture.transitionLayout(
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::cout << "TRANSITIONED\n";

    return Material{
        device,
        descriptorManager,
        swapChain,
        &texture,
        texture,
        vertexShader,
        fragmentShader,
        MaterialUsage::ShadowMap};
}

DirectionalLight::DirectionalLight(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain)
    : mDevice{device},
      mSwapChain{swapChain},
      mImageAvailableSemaphore{static_cast<vk::Device>(mDevice).createSemaphore({})},
      mRenderFinishedSemaphore{static_cast<vk::Device>(mDevice).createSemaphore({})},
      mMaterial{createMaterial(mDevice, descriptorManager, mSwapChain)}
{
}

void DirectionalLight::createCommandBuffers(std::vector<Model>& models)
{
    vk::CommandBufferAllocateInfo commandBufferInfo(
        mDevice.commandPool(), vk::CommandBufferLevel::ePrimary, mSwapChain.imageCount());

    mCommandBuffers = static_cast<vk::Device>(mDevice).allocateCommandBuffers(commandBufferInfo);

    size_t framebufferIndex = 0;
    for (vk::CommandBuffer commandBuffer : mCommandBuffers) {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
        beginInfo.pInheritanceInfo = nullptr;
        commandBuffer.begin(beginInfo);

        framebufferIndex++;
        commandBuffer.end();
    }
}

void DirectionalLight::drawFrame()
{
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
