
#include "../Include/Framebuffer.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

vk::Framebuffer createFramebuffer(
    vk::Device device,
    vk::ImageView imageView,
    vk::ImageView depthImageView,
    vk::Extent2D swapChainExtent,
    vk::RenderPass renderPass)
{
    std::array<vk::ImageView, 2> attachments = {imageView, depthImageView};

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    vk::Framebuffer framebuffer = device.createFramebuffer(framebufferInfo, nullptr);
    return framebuffer;
}

Framebuffer::Framebuffer(
    Device& device,
    vk::ImageView imageView,
    vk::ImageView depthImageView,
    vk::Extent2D swapChainExtent,
    RenderPass& renderPass)
    : mDevice{device},
      mRenderPass{renderPass},
      mFramebuffer{
          createFramebuffer(device, imageView, depthImageView, swapChainExtent, renderPass)}
{
}

Framebuffer::~Framebuffer()
{
}
