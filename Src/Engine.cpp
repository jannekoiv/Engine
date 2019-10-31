#include "../Include/Engine.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>

std::vector<vk::Framebuffer> createFrameBuffers(
    Device& device, SwapChain& swapChain, vk::ImageView depthImageView, RenderPass& renderPass)
{
    std::vector<vk::Framebuffer> frameBuffers(swapChain.imageViews().size());

    for (size_t i = 0; i < swapChain.imageViews().size(); i++) {
        std::vector<vk::ImageView> attachments = {swapChain.imageViews()[i], depthImageView};

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChain.extent().width;
        framebufferInfo.height = swapChain.extent().height;
        framebufferInfo.layers = 1;

        frameBuffers[i] =
            static_cast<vk::Device>(device).createFramebuffer(framebufferInfo, nullptr);
    }
    return frameBuffers;
}

Engine::Engine(const InitInfo& initInfo, GLFWwindow* window)
    : mDevice(
          window,
          initInfo.enableValidationLayers,
          initInfo.validationLayers,
          initInfo.deviceExtensions),
      mSwapChain(mDevice, vk::Extent2D(initInfo.width, initInfo.height)),
      mRenderPass(mDevice, mSwapChain.format()),
      mDepthImage(
          mDevice,
          vk::Extent3D(mSwapChain.extent()),
          mRenderPass.depthAttachmentFormat(),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eDepthStencilAttachment,
          vk::MemoryPropertyFlagBits::eDeviceLocal),
      mFrameBuffers(createFrameBuffers(mDevice, mSwapChain, mDepthImage.view(), mRenderPass))
{
}

Engine::~Engine()
{
}

int timeInMilliseconds()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto timeMillis =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return static_cast<int>(timeMillis);
}
