#include "includes.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static std::array<bool, GLFW_KEY_LAST> keyboard_state{};
    if (action == GLFW_PRESS) {
        keyboard_state[key] = true;
    } else if (action == GLFW_RELEASE) {
        keyboard_state[key] = false;
    }

    auto& context = *static_cast<Context*>(glfwGetWindowUserPointer(window));
    context.send_keys(keyboard_state);
}

GLFWwindow* init_window(const int width, const int height, void* user_pointer)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        width, height, "Totaalinen Yliruletus Rendering Engine", nullptr, nullptr);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowUserPointer(window, user_pointer);
    return window;
}

void Context::set_cursor_callback(GLFWcursorposfun callback)
{
    glfwSetCursorPosCallback(_window, callback);
}

vk::CommandPool create_command_pool(Device& device)
{
    vk::CommandPoolCreateInfo info(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        device.queue_family_indices().graphics);
    vk::CommandPool command_pool =
        static_cast<vk::Device>(device).createCommandPool(info);
    return command_pool;
}

std::vector<vk::CommandBuffer> create_command_buffers(
    vk::Device device, SwapChain& swap_chain, vk::CommandPool command_pool)
{
    vk::CommandBufferAllocateInfo command_buffer_info(
        command_pool,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(swap_chain.image_count()));

    std::vector<vk::CommandBuffer> command_buffers =
        device.allocateCommandBuffers(command_buffer_info);

    return command_buffers;
}

vk::RenderPass create_render_pass(Device& device, SwapChain& swap_chain)
{
    std::vector<vk::AttachmentDescription> attachments{};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<vk::AttachmentReference> color_attachment_refs{
        {{0, vk::ImageLayout::eColorAttachmentOptimal}}};

    vk::AttachmentDescription color_attachment{};
    color_attachment.format = swap_chain.format();
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
    depth_attachment.format = find_optimal_depth_attachment_format(device);
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
        static_cast<vk::Device>(device).createRenderPass(info, nullptr);
    return render_pass;
}

static std::vector<vk::Framebuffer> create_framebuffers(
    vk::Device device,
    SwapChain& swap_chain,
    Texture& depth_texture,
    vk::RenderPass render_pass)
{
    std::vector<vk::Framebuffer> framebuffers(swap_chain.image_count());

    for (int i = 0; i < swap_chain.image_count(); i++) {
        std::vector<vk::ImageView> attachments{
            swap_chain.image_view(i), depth_texture.image_view()};

        vk::FramebufferCreateInfo info{};
        info.renderPass = render_pass;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.width = swap_chain.extent().width;
        info.height = swap_chain.extent().height;
        info.layers = 1;

        vk::Framebuffer framebuffer = device.createFramebuffer(info, nullptr);
        framebuffers[i] = framebuffer;
    }

    return framebuffers;
}

Context::Context(const int width, const int height)
    : _window{init_window(width, height, this)},
      _device{_window, true},
      _swap_chain{_device},
      _depth_texture{
          _device,
          vk::ImageViewType::e2D,
          1,
          vk::Extent3D{_swap_chain.extent().width, _swap_chain.extent().height, 1},
          find_optimal_depth_attachment_format(_device),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eDepthStencilAttachment |
              vk::ImageUsageFlagBits::eSampled,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge},
      _command_pool(create_command_pool(_device)),
      _command_buffers{create_command_buffers(_device, _swap_chain, _command_pool)},
      _image_available_semaphore{static_cast<vk::Device>(_device).createSemaphore({})},
      _render_finished_semaphore{static_cast<vk::Device>(_device).createSemaphore({})},
      _image_index{0},
      _render_pass{create_render_pass(_device, _swap_chain)},
      _framebuffers{
          create_framebuffers(_device, _swap_chain, _depth_texture, _render_pass)},
      _input{}
{
    std::cout << "Context initialized\n";
}

Context::~Context()
{
    static_cast<vk::Device>(_device).destroyCommandPool(_command_pool);
    static_cast<vk::Device>(_device).destroySemaphore(_render_finished_semaphore);
    static_cast<vk::Device>(_device).destroySemaphore(_image_available_semaphore);
    static_cast<vk::Device>(_device).destroyRenderPass(_render_pass);
    for (auto framebuffer : _framebuffers) {
        static_cast<vk::Device>(_device).destroyFramebuffer(framebuffer);
    }
}

void Context::begin_frame()
{
    static_cast<void>(static_cast<vk::Device>(_device).acquireNextImageKHR(
        _swap_chain,
        std::numeric_limits<uint64_t>::max(),
        _image_available_semaphore,
        nullptr,
        &_image_index));

    vk::CommandBuffer command_buffer = _command_buffers[_image_index];

    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    command_buffer.begin(begin_info);
}

void Context::end_frame()
{
    vk::CommandBuffer command_buffer = _command_buffers[_image_index];
    command_buffer.end();

    vk::SubmitInfo submit_info;
    vk::PipelineStageFlags wait_stages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &_image_available_semaphore;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &_render_finished_semaphore;

    _device.graphics_queue().submit(submit_info, nullptr);

    vk::PresentInfoKHR present_info;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_finished_semaphore;
    present_info.swapchainCount = 1;
    auto swapChain = static_cast<vk::SwapchainKHR>(_swap_chain);
    present_info.pSwapchains = &swapChain;
    present_info.pImageIndices = &_image_index;
    present_info.pResults = nullptr;

    static_cast<void>(_device.present_queue().presentKHR(present_info));
    static_cast<vk::Device>(_device).waitIdle();
}

void Context::clear_frame()
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = _render_pass;
    render_pass_info.framebuffer = _framebuffers[_image_index];
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = _swap_chain.extent();
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    command_buffer().beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer().endRenderPass();
}

const Input& Context::input()
{
    double xpos = 0.0f;
    double ypos = 0.0f;
    glfwGetCursorPos(_window, &xpos, &ypos);
    static double prev_xpos = xpos;
    static double prev_ypos = ypos;

    _input.mouse_x = static_cast<float>(xpos);
    _input.mouse_y = static_cast<float>(ypos);
    _input.mouse_x_offset = _input.mouse_x - static_cast<float>(prev_xpos);
    _input.mouse_y_offset = _input.mouse_y - static_cast<float>(prev_ypos);
    prev_xpos = xpos;
    prev_ypos = ypos;
    return _input;
}

void Context::hide_cursor(bool hide)
{
    if (hide) {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    } else {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
}

void Context::send_keys(std::array<bool, GLFW_KEY_LAST>& keys)
{
    _input.keys = keys;
}
