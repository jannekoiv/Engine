
#include "includes.h"

// std::vector<vk::CommandBuffer> create_command_buffers(Context& context)
// {
//     vk::CommandBufferAllocateInfo command_buffer_info(
//         context.device().command_pool(),
//         vk::CommandBufferLevel::ePrimary,
//         static_cast<uint32_t>(context.swap_chain().image_count()));

//     std::vector<vk::CommandBuffer> command_buffers =
//         static_cast<vk::Device>(context).allocateCommandBuffers(command_buffer_info);

//     return command_buffers;
// }

vk::RenderPass create_render_pass(Context& context)
{
    std::vector<vk::AttachmentDescription> attachments{};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<vk::AttachmentReference> color_attachment_refs{
        {{0, vk::ImageLayout::eColorAttachmentOptimal}}};

    vk::AttachmentDescription color_attachment{};
    color_attachment.format = context.swap_chain().format();
    color_attachment.samples = vk::SampleCountFlagBits::e1;
    color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    color_attachment.initialLayout = vk::ImageLayout::eUndefined;
    color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    subpass.colorAttachmentCount = static_cast<uint32_t>(color_attachment_refs.size());
    subpass.pColorAttachments = color_attachment_refs.data();

    attachments.push_back(color_attachment);

    vk::AttachmentReference depth_attachment_ref{};

    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    vk::AttachmentDescription depth_attachment{};
    depth_attachment.format = find_optimal_depth_attachment_format(context.device());
    depth_attachment.samples = vk::SampleCountFlagBits::e1;
    depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    depth_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
    depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    attachments.push_back(depth_attachment);

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = {};
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo info;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    vk::RenderPass render_pass =
        static_cast<vk::Device>(context.device()).createRenderPass(info, nullptr);
    return render_pass;
}

static std::vector<vk::Framebuffer> create_framebuffers(
    Context& context, vk::RenderPass render_pass)
{
    std::vector<vk::Framebuffer> framebuffers(context.swap_chain().image_count());

    for (int i = 0; i < context.swap_chain().image_count(); i++) {
        std::vector<vk::ImageView> attachments{
            context.swap_chain().image_view(i), context.depth_texture().image_view()};

        vk::FramebufferCreateInfo info{};
        info.renderPass = render_pass;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.width = context.swap_chain().extent().width;
        info.height = context.swap_chain().extent().height;
        info.layers = 1;

        vk::Framebuffer framebuffer =
            static_cast<vk::Device>(context.device()).createFramebuffer(info, nullptr);
        framebuffers[i] = framebuffer;
    }

    return framebuffers;
}

Renderer::Renderer(Context& context)
    : _context{context},
      _render_pass{create_render_pass(context)},
      _framebuffers{create_framebuffers(context, _render_pass)}
// _command_buffers{create_command_buffers(context)},
// _image_available_semaphore{static_cast<vk::Device>(context).createSemaphore({})},
// _render_finished_semaphore{static_cast<vk::Device>(context).createSemaphore({})}
{
    std::cout << "Renderer initialized\n";
}

Renderer::~Renderer()
{
    static_cast<vk::Device>(_context.device()).destroyRenderPass(_render_pass);
    for (auto framebuffer : _framebuffers) {
        static_cast<vk::Device>(_context.device()).destroyFramebuffer(framebuffer);
    }
    // static_cast<vk::Device>(_context).destroySemaphore(_render_finished_semaphore);
    // static_cast<vk::Device>(_context).destroySemaphore(_image_available_semaphore);
}

static void* aligned_malloc(size_t size, size_t alignment)
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

static void aligned_free(void* data)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(data);
#else
    free(data);
#endif
}

static void clear_color(
    Device& device, SwapChain& swap_chain, int index, vk::CommandBuffer command_buffer)
{
    vk::ClearColorValue clear_color{std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f}};

    vk::ImageSubresourceRange image_range{};
    image_range.aspectMask = vk::ImageAspectFlagBits::eColor;
    image_range.baseMipLevel = 0;
    image_range.levelCount = 1;
    image_range.baseArrayLayer = 0;
    image_range.layerCount = 1;

    vk::ImageMemoryBarrier present_to_clear_barrier{};
    present_to_clear_barrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    present_to_clear_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    present_to_clear_barrier.oldLayout = vk::ImageLayout::eUndefined;
    present_to_clear_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    present_to_clear_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_to_clear_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_to_clear_barrier.image = swap_chain.image(index);
    present_to_clear_barrier.subresourceRange = image_range;

    // Change layout of image to be optimal for presenting
    vk::ImageMemoryBarrier clear_to_present_barrier{};
    clear_to_present_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    clear_to_present_barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    clear_to_present_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    clear_to_present_barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    clear_to_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clear_to_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clear_to_present_barrier.image = swap_chain.image(index);
    clear_to_present_barrier.subresourceRange = image_range;

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        {},
        {},
        {present_to_clear_barrier});

    command_buffer.clearColorImage(
        swap_chain.image(index),
        vk::ImageLayout::eTransferDstOptimal,
        clear_color,
        image_range);

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clear_to_present_barrier});
}

static void clear_pass(
    vk::CommandBuffer command_buffer,
    vk::RenderPass render_pass,
    vk::Framebuffer frame_buffer,
    vk::Extent2D swap_chain_extent)
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = frame_buffer;
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = swap_chain_extent;
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.endRenderPass();
}

// static void draw_skybox_pass(
//     vk::CommandBuffer command_buffer,
//     int framebuffer_index,
//     vk::Extent2D swap_chain_extent,
//     Skybox& skybox)
// {
//     vk::RenderPassBeginInfo render_pass_info;
//     render_pass_info.renderPass = skybox.pipeline().framebuffer_set().render_pass();
//     render_pass_info.framebuffer =
//         skybox.pipeline().framebuffer_set().frame_buffer(framebuffer_index);
//     render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
//     render_pass_info.renderArea.extent = swap_chain_extent;

//     command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
//     command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skybox.pipeline());
//     command_buffer.bindVertexBuffers(0, {skybox.vertex_buffer()}, {0});
//     command_buffer.bindIndexBuffer(skybox.index_buffer(), 0, vk::IndexType::eUint32);

//     command_buffer.bindDescriptorSets(
//         vk::PipelineBindPoint::eGraphics,
//         skybox.pipeline().layout(),
//         0,
//         {skybox.descriptor_set()},
//         nullptr);

//     command_buffer.bindDescriptorSets(
//         vk::PipelineBindPoint::eGraphics,
//         skybox.pipeline().layout(),
//         1,
//         {skybox.pipeline().descriptor_set()},
//         nullptr);

//     command_buffer.drawIndexed(36, 1, 0, 0, 0);

//     command_buffer.endRenderPass();
// }

// uint32_t Renderer::begin_frame()
// {
//     uint32_t image_index = 0;
//     static_cast<void>(static_cast<vk::Device>(_context).acquireNextImageKHR(
//         _context.swap_chain(),
//         std::numeric_limits<uint64_t>::max(),
//         _image_available_semaphore,
//         nullptr,
//         &image_index));

//     vk::CommandBuffer command_buffer = _command_buffers[image_index];

//     vk::CommandBufferBeginInfo begin_info{};
//     begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
//     command_buffer.begin(begin_info);
//     return image_index;
// }

// void Renderer::end_frame(uint32_t image_index)
// {
//     vk::CommandBuffer command_buffer = _command_buffers[image_index];
//     command_buffer.end();

//     vk::SubmitInfo submit_info;
//     vk::PipelineStageFlags wait_stages[] = {
//         vk::PipelineStageFlagBits::eColorAttachmentOutput};
//     submit_info.waitSemaphoreCount = 1;
//     submit_info.pWaitSemaphores = &_image_available_semaphore;
//     submit_info.pWaitDstStageMask = wait_stages;

//     submit_info.commandBufferCount = 1;
//     submit_info.pCommandBuffers = &command_buffer;

//     submit_info.signalSemaphoreCount = 1;
//     submit_info.pSignalSemaphores = &_render_finished_semaphore;

//     _context.device().graphics_queue().submit(submit_info, nullptr);

//     vk::PresentInfoKHR present_info;
//     present_info.waitSemaphoreCount = 1;
//     present_info.pWaitSemaphores = &_render_finished_semaphore;
//     present_info.swapchainCount = 1;
//     auto swapChain = static_cast<vk::SwapchainKHR>(_context.swap_chain());
//     present_info.pSwapchains = &swapChain;
//     present_info.pImageIndices = &image_index;
//     present_info.pResults = nullptr;

//     static_cast<void>(_context.device().present_queue().presentKHR(present_info));
//     static_cast<vk::Device>(_context).waitIdle();
// }

void Renderer::clear()
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = _render_pass;
    render_pass_info.framebuffer = _framebuffers[_context.image_index()];
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = _context.swap_chain().extent();
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    vk::CommandBuffer command_buffer = _context.command_buffer();

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.endRenderPass();
}
