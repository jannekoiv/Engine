
#include "../Include/FramebufferSet.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "../Include/Image.h"
#include "../Include/SwapChain.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

vk::RenderPass createRenderPass(Device& device, SwapChain& swapChain, vk::AttachmentLoadOp loadOp)
{
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = swapChain.format();
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = loadOp;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    if (loadOp == vk::AttachmentLoadOp::eLoad) {
        colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
    }

    vk::AttachmentReference colorAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthAttachmentFormat(device);
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = loadOp;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    if (loadOp == vk::AttachmentLoadOp::eLoad) {
        depthAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
    }

    vk::AttachmentReference depthAttachmentRef{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = {};
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    std::vector<vk::AttachmentDescription> attachments = {colorAttachment, depthAttachment};

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vk::RenderPass renderPass =
        static_cast<vk::Device>(device).createRenderPass(renderPassInfo, nullptr);
    return renderPass;
}

std::vector<vk::Framebuffer> createFramebuffers(
    Device& device, SwapChain& swapChain, Image& depthImage, vk::RenderPass renderPass)
{
    std::vector<vk::Framebuffer> framebuffers(swapChain.imageCount());

    for (int i = 0; i < swapChain.imageCount(); i++) {
        std::array<vk::ImageView, 2> attachments = {swapChain.imageView(i), depthImage.view()};

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChain.extent().width;
        framebufferInfo.height = swapChain.extent().height;
        framebufferInfo.layers = 1;

        vk::Framebuffer framebuffer =
            static_cast<vk::Device>(device).createFramebuffer(framebufferInfo, nullptr);
        framebuffers[i] = framebuffer;
    }

    return framebuffers;
}

FramebufferSet::FramebufferSet(
    Device& device, SwapChain& swapChain, Image& depthImage, vk::AttachmentLoadOp loadOp)
    : mRenderPass{createRenderPass(device, swapChain, loadOp)},
      mFramebuffers{createFramebuffers(device, swapChain, depthImage, mRenderPass)}
{
}

FramebufferSet::~FramebufferSet()
{
}
