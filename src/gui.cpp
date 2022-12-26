#include "includes.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

static vk::DescriptorPool create_descriptor_pool(vk::Device device)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes = {
        {vk::DescriptorType::eUniformBuffer, 1},
        {vk::DescriptorType::eCombinedImageSampler, 1}};

    vk::DescriptorPoolCreateInfo info{};
    info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    info.maxSets = 1;
    info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    info.pPoolSizes = pool_sizes.data();

    vk::DescriptorPool pool = device.createDescriptorPool(info, nullptr);
    return pool;
}

void check_result(VkResult result)
{
    if (result != VK_SUCCESS) {
        std::cout << "GUI error!\n";
    }
}

static vk::RenderPass create_render_pass(Device& device, vk::Format swap_chain_format)
{
    std::vector<vk::AttachmentDescription> attachments{};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<vk::AttachmentReference> color_attachment_refs{
        {{0, vk::ImageLayout::eColorAttachmentOptimal}}};

    vk::AttachmentDescription color_attachment{};
    color_attachment.format = swap_chain_format;
    color_attachment.samples = vk::SampleCountFlagBits::e1;
    color_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    color_attachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
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
    depth_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
    depth_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depth_attachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
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
    Device& device,
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

        vk::Framebuffer framebuffer =
            static_cast<vk::Device>(device).createFramebuffer(info, nullptr);
        framebuffers[i] = framebuffer;
    }

    return framebuffers;
}

Gui::Gui(Context& context) : _context{context}
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    auto& device = context.device();
    _descriptor_pool = create_descriptor_pool(device);

    ImGui_ImplGlfw_InitForVulkan(context.window(), true);
    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = device.instance();
    info.PhysicalDevice = device.physical_device();
    info.Device = static_cast<vk::Device>(device);
    info.QueueFamily = device.queue_family_indices().graphics;
    info.Queue = device.graphics_queue();
    info.DescriptorPool = _descriptor_pool;
    info.MinImageCount = context.swap_chain().image_count();
    info.ImageCount = context.swap_chain().image_count();
    info.CheckVkResultFn = check_result;

    _render_pass = create_render_pass(context.device(), context.swap_chain().format());

    _framebuffers = create_framebuffers(
        device, _context.swap_chain(), _context.depth_texture(), _render_pass);

    ImGui_ImplVulkan_Init(&info, _render_pass);

    auto command_buffer = device.create_and_begin_command_buffer(context.command_pool());
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    device.flush_and_free_command_buffer(context.command_pool(), command_buffer);
}

Gui::~Gui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    static_cast<vk::Device>(_context.device()).destroyDescriptorPool(_descriptor_pool);
    static_cast<vk::Device>(_context.device()).destroyRenderPass(_render_pass);
    for (auto framebuffer : _framebuffers) {
        static_cast<vk::Device>(_context.device()).destroyFramebuffer(framebuffer);
    }
}

void Gui::draw(const std::string& text)
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = _render_pass;
    render_pass_info.framebuffer = _framebuffers[_context.image_index()];
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = _context.swap_chain().extent();

    auto command_buffer = _context.command_buffer();

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool tool_active = false;

    ImGui::SetNextWindowSize(ImVec2{1000, 500});
    ImGui::SetNextWindowPos(ImVec2{0, 0});

    ImGui::Begin(
        "Hoo",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoResize);

    ImGui::SetWindowFontScale(2.0f);

    ImGui::Text("%s", text.data());

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

    command_buffer.endRenderPass();
}
