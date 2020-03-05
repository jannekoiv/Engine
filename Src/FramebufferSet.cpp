
#include "../Include/FramebufferSet.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

FramebufferSet::FramebufferSet(FramebufferSet&& rhs)
    : mDevice{rhs.mDevice}, mRenderPass{rhs.mRenderPass}, mFramebuffers{rhs.mFramebuffers}
{
    rhs.mRenderPass = nullptr;
    rhs.mFramebuffers.clear();
}

vk::RenderPass createRenderPass(Device& device, vk::Format swapChainFormat, MaterialUsage materialUsage)
{
    std::vector<vk::AttachmentDescription> attachments{};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<vk::AttachmentReference> colorAttachmentRefs{{{0, vk::ImageLayout::eColorAttachmentOptimal}}};

    if (materialUsage != MaterialUsage::ShadowMap) {
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;

        if (materialUsage == MaterialUsage::Clear) {
            colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        } else {
            colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
        }

        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        if (materialUsage == MaterialUsage::Clear) {
            colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        } else {
            colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
        }

        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        subpass.colorAttachmentCount = colorAttachmentRefs.size();
        subpass.pColorAttachments = colorAttachmentRefs.data();

        attachments.push_back(colorAttachment);
    }

    if (materialUsage != MaterialUsage::Quad) {
        vk::AttachmentReference depthAttachmentRef{};

        if (materialUsage != MaterialUsage::ShadowMap) {
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        } else {
            depthAttachmentRef.attachment = 0;
            depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }

        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        vk::AttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthAttachmentFormat(device);
        depthAttachment.samples = vk::SampleCountFlagBits::e1;

        if (materialUsage == MaterialUsage::Clear) {
            depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        } else {
            depthAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
        }

        if (materialUsage == MaterialUsage::ShadowMap) {
            depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        }

        depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        if (materialUsage == MaterialUsage::Clear) {
            depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
        } else {
            depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }
        depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        if (materialUsage == MaterialUsage::ShadowMap) {
            depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            depthAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }

        attachments.push_back(depthAttachment);
    }

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = {};
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    std::cout << "attahcment count " << renderPassInfo.attachmentCount << "\n";
    vk::RenderPass renderPass = static_cast<vk::Device>(device).createRenderPass(renderPassInfo, nullptr);
    return renderPass;
}

std::vector<vk::Framebuffer> createFramebuffers(
    Device& device, SwapChain& swapChain, Texture* depthTexture, vk::RenderPass renderPass, MaterialUsage materialUsage)
{
    if (materialUsage == MaterialUsage::ShadowMap) {
        std::vector<vk::ImageView> attachments = {depthTexture->imageView()};

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChain.extent().width;
        framebufferInfo.height = swapChain.extent().height;
        framebufferInfo.layers = 1;

        vk::Framebuffer framebuffer = static_cast<vk::Device>(device).createFramebuffer(framebufferInfo, nullptr);
        return {framebuffer};
    } else {
        std::vector<vk::Framebuffer> framebuffers(swapChain.imageCount());

        for (int i = 0; i < swapChain.imageCount(); i++) {
            std::vector<vk::ImageView> attachments = {swapChain.imageView(i)};
            if (depthTexture) {
                attachments.push_back(depthTexture->imageView());
            }

            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChain.extent().width;
            framebufferInfo.height = swapChain.extent().height;
            framebufferInfo.layers = 1;

            vk::Framebuffer framebuffer = static_cast<vk::Device>(device).createFramebuffer(framebufferInfo, nullptr);
            framebuffers[i] = framebuffer;
        }

        return framebuffers;
    }
}

FramebufferSet::FramebufferSet(Device& device, SwapChain& swapChain, Texture* depthTexture, MaterialUsage materialUsage)
    : mDevice{device},
      mRenderPass{createRenderPass(mDevice, swapChain.format(), materialUsage)},
      mFramebuffers{createFramebuffers(mDevice, swapChain, depthTexture, mRenderPass, materialUsage)}
{
}

FramebufferSet::~FramebufferSet()
{
    for (auto frameBuffer : mFramebuffers) {
        static_cast<vk::Device>(mDevice).destroyFramebuffer(frameBuffer);
    }

    static_cast<vk::Device>(mDevice).destroyRenderPass(mRenderPass);
}
