
#include "framebuffer_set.h"
#include "base.h"
#include "device.h"
#include "swap_chain.h"
#include "texture.h"

FramebufferSet::FramebufferSet(FramebufferSet&& rhs)
    : _device{rhs._device}, _render_pass{rhs._render_pass}, _frame_buffers{rhs._frame_buffers}
{
    rhs._render_pass = nullptr;
    rhs._frame_buffers.clear();
}

vk::RenderPass create_render_pass(
    Device& device, vk::Format swap_chain_format, const nlohmann::json& json)
{
    std::vector<vk::AttachmentDescription> attachments{};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<vk::AttachmentReference> color_attachment_refs{{{0, vk::ImageLayout::eColorAttachmentOptimal}}};

    std::cout << "json size " << json.size() << "\n";
    if (json.contains("usage")) {
        std::cout << "usage found\n";
    } else {
        std::cout << "usage not found\n";
    }

    if (json["usage"] != "ShadowMap") {
        vk::AttachmentDescription color_attachment{};
        color_attachment.format = swap_chain_format;
        color_attachment.samples = vk::SampleCountFlagBits::e1;

        if (json["usage"] == "Clear") {
            color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        } else {
            color_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
        }

        color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        if (json["usage"] == "Clear") {
            color_attachment.initialLayout = vk::ImageLayout::eUndefined;
        } else {
            color_attachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
        }

        color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        subpass.colorAttachmentCount = static_cast<uint32_t>(color_attachment_refs.size());
        subpass.pColorAttachments = color_attachment_refs.data();

        attachments.push_back(color_attachment);
    }

    if (json["usage"] != "Quad") {
        vk::AttachmentReference depth_attachment_ref{};

        if (json["usage"] != "ShadowMap") {
            depth_attachment_ref.attachment = 1;
            depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        } else {
            depth_attachment_ref.attachment = 0;
            depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }

        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        vk::AttachmentDescription depth_attachment{};
        depth_attachment.format = find_depth_attachment_optimal(device);
        depth_attachment.samples = vk::SampleCountFlagBits::e1;

        if (json["usage"] == "Clear") {
            depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        } else {
            depth_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
        }

        if (json["usage"] == "ShadowMap") {
            depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        }

        depth_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        if (json["usage"] == "Clear") {
            depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
        } else {
            depth_attachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }
        depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        if (json["usage"] == "ShadowMap") {
            depth_attachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            depth_attachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }

        attachments.push_back(depth_attachment);
    }

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = {};
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo info;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    std::cout << "attahcment count " << info.attachmentCount << "\n";
    vk::RenderPass render_pass = static_cast<vk::Device>(device).createRenderPass(info, nullptr);
    return render_pass;
}

std::vector<vk::Framebuffer> create_frame_buffers(
    Device& device,
    SwapChain& swap_chain,
    Texture* depth_texture,
    vk::RenderPass render_pass,
    const nlohmann::json& json)
{
    if (json["usage"] == "ShadowMap") {
        std::vector<vk::ImageView> attachments = {depth_texture->image_view()};

        vk::FramebufferCreateInfo info{};
        info.renderPass = render_pass;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.width = swap_chain.extent().width;
        info.height = swap_chain.extent().height;
        info.layers = 1;

        vk::Framebuffer framebuffer = static_cast<vk::Device>(device).createFramebuffer(info, nullptr);
        return {framebuffer};
    } else {
        std::vector<vk::Framebuffer> framebuffers(swap_chain.image_count());

        for (int i = 0; i < swap_chain.image_count(); i++) {
            std::vector<vk::ImageView> attachments = {swap_chain.image_view(i)};
            if (depth_texture) {
                attachments.push_back(depth_texture->image_view());
            }

            vk::FramebufferCreateInfo info{};
            info.renderPass = render_pass;
            info.attachmentCount = static_cast<uint32_t>(attachments.size());
            info.pAttachments = attachments.data();
            info.width = swap_chain.extent().width;
            info.height = swap_chain.extent().height;
            info.layers = 1;

            vk::Framebuffer framebuffer = static_cast<vk::Device>(device).createFramebuffer(info, nullptr);
            framebuffers[i] = framebuffer;
        }

        return framebuffers;
    }
}

FramebufferSet::FramebufferSet(
    Device& device,
    SwapChain& swap_chain,
    Texture* depth_texture,
    const nlohmann::json& json)
    : _device{device},
      _render_pass{create_render_pass(_device, swap_chain.format(), json)},
      _frame_buffers{create_frame_buffers(device, swap_chain, depth_texture, _render_pass, json)}
{
}

FramebufferSet::~FramebufferSet()
{
    for (auto buffer : _frame_buffers) {
        static_cast<vk::Device>(_device).destroyFramebuffer(buffer);
    }

    static_cast<vk::Device>(_device).destroyRenderPass(_render_pass);
}
