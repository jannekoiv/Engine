#include "quad.h"
#include "device.h"
#include "framebuffer_set.h"
#include "swap_chain.h"

// static Buffer create_uniform_buffer(Device& device)
// {
//     return Buffer(
//         device,
//         sizeof(QuadUniform),
//         vk::BufferUsageFlagBits::eUniformBuffer,
//         vk::MemoryPropertyFlagBits::eHostVisible |
//             vk::MemoryPropertyFlagBits::eHostCoherent);
// }

static Buffer create_vertex_buffer(Device& device)
{
    std::array<QuadVertex, 6> vertices = {
        {{{-1.0f, 0.2f}, 0xffff0000},
         {{-0.5f, -0.4f}, 0xff00ff00},
         {{0.0f, 0.2f}, 0xff0000ff},
         {{0.0f, 0.2f}, 0xffff0000},
         {{0.5f, -0.4f}, 0xff00ff00},
         {{1.0f, 0.2f}, 0xff0000ff}}};

    vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

    Buffer staging_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    staging_buffer.unmap_memory();

    Buffer vertex_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    staging_buffer.copy(vertex_buffer);

    return vertex_buffer;
}

// static DescriptorSet create_descriptor_set(
//     DescriptorManager& descriptor_manager, vk::Buffer uniform_buffer)
// {
//     std::vector<vk::DescriptorSetLayoutBinding> bindings = {
//         {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

//     DescriptorSet descriptor_set = descriptor_manager.create_descriptor_set(bindings);

//     vk::DescriptorBufferInfo buffer_info;
//     buffer_info.buffer = uniform_buffer;
//     buffer_info.offset = 0;
//     buffer_info.range = sizeof(QuadUniform);

//     descriptor_set.write_descriptors({{0, 0, 1, &buffer_info}});
//     return descriptor_set;
// }

static std::vector<vk::CommandBuffer> create_command_buffers(
    Device& device, SwapChain& swap_chain)
{
    vk::CommandBufferAllocateInfo command_buffer_info(
        device.command_pool(),
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(swap_chain.image_count()));

    std::vector<vk::CommandBuffer> command_buffers =
        static_cast<vk::Device>(device).allocateCommandBuffers(command_buffer_info);

    return command_buffers;
}

static vk::PipelineLayout create_pipeline_layout(Device& device)
{
    vk::PipelineLayoutCreateInfo info{};
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;

    vk::PipelineLayout layout =
        static_cast<vk::Device>(device).createPipelineLayout(info);
    return layout;
}

static vk::ShaderModule create_shader_from_file(vk::Device device, std::string filename)
{
    auto code = read_file(filename);
    vk::ShaderModuleCreateInfo info{};
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    vk::ShaderModule module = device.createShaderModule(info, nullptr);
    return module;
}

static std::vector<vk::PipelineShaderStageCreateInfo> create_shader_stages(Device& device)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};

    vk::PipelineShaderStageCreateInfo vertex_shader_stage;
    vertex_shader_stage.stage = vk::ShaderStageFlagBits::eVertex;
    vertex_shader_stage.module = create_shader_from_file(device, "vert.spv");
    vertex_shader_stage.pName = "main";
    shader_stages.push_back(vertex_shader_stage);

    vk::PipelineShaderStageCreateInfo fragment_shader_stage;
    fragment_shader_stage.stage = vk::ShaderStageFlagBits::eFragment;
    fragment_shader_stage.module = create_shader_from_file(device, "frag.spv");
    fragment_shader_stage.pName = "main";
    shader_stages.push_back(fragment_shader_stage);

    return shader_stages;
}

static vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_info()
{
    vk::PipelineInputAssemblyStateCreateInfo info{};
    info.topology = vk::PrimitiveTopology::eTriangleList;
    info.primitiveRestartEnable = false;
    return info;
}

static vk::PipelineVertexInputStateCreateInfo vertex_input_state_info(
    const vk::VertexInputBindingDescription& binding_description,
    const std::vector<vk::VertexInputAttributeDescription>& attribute_description)
{
    vk::PipelineVertexInputStateCreateInfo info{};
    info.vertexBindingDescriptionCount = 1;
    info.pVertexBindingDescriptions = &binding_description;
    info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_description.size());
    info.pVertexAttributeDescriptions = attribute_description.data();
    return info;
}

struct ViewportStateCreateInfo {
    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
    vk::PipelineViewportStateCreateInfo info;
};

static ViewportStateCreateInfo viewport_state_info(vk::Extent2D swap_chain_extent)
{
    ViewportStateCreateInfo info{};
    info.viewports.resize(1);
    info.scissors.resize(1);

    info.viewports[0].x = 0.0f;
    info.viewports[0].y = 0.0f;
    info.viewports[0].width = static_cast<float>(swap_chain_extent.width);
    info.viewports[0].height = static_cast<float>(swap_chain_extent.height);
    info.viewports[0].minDepth = 0.0f;
    info.viewports[0].maxDepth = 1.0f;

    info.scissors[0].offset = vk::Offset2D(0, 0);
    info.scissors[0].extent = swap_chain_extent;

    info.info.viewportCount = 1;
    info.info.pViewports = &info.viewports[0];
    info.info.scissorCount = 1;
    info.info.pScissors = &info.scissors[0];

    return info;
}

static vk::PipelineRasterizationStateCreateInfo rasterization_state_info()
{
    vk::PipelineRasterizationStateCreateInfo info{};

    info.polygonMode = vk::PolygonMode::eFill;
    info.lineWidth = 1.0f;
    info.cullMode = vk::CullModeFlagBits::eNone;
    info.frontFace = vk::FrontFace::eCounterClockwise;

    return info;
}

static vk::PipelineMultisampleStateCreateInfo multi_sample_state_info()
{
    vk::PipelineMultisampleStateCreateInfo info{};
    info.sampleShadingEnable = false;
    info.rasterizationSamples = vk::SampleCountFlagBits::e1;
    info.minSampleShading = 1.0;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = false;
    info.alphaToOneEnable = false;
    return info;
}

static vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_info()
{
    vk::PipelineDepthStencilStateCreateInfo info{};
    info.depthTestEnable = true;
    info.depthWriteEnable = true;
    info.depthCompareOp = vk::CompareOp::eGreater;
    return info;
}

struct ColorBlendStateCreateInfo {
    std::vector<vk::PipelineColorBlendAttachmentState> attachments;
    vk::PipelineColorBlendStateCreateInfo info;
};

static ColorBlendStateCreateInfo color_blend_state_info()
{
    ColorBlendStateCreateInfo info{};
    info.attachments.resize(1);

    info.attachments[0].colorWriteMask = vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eB;
    info.attachments[0].blendEnable = false;

    info.info.logicOpEnable = false;
    info.info.logicOp = vk::LogicOp::eCopy;
    info.info.attachmentCount = static_cast<uint32_t>(info.attachments.size());
    info.info.pAttachments = info.attachments.data();

    return info;
}

static vk::Pipeline create_pipeline(
    Device& device,
    FramebufferSet& framebuffer_set,
    vk::VertexInputBindingDescription binding_description,
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions,
    vk::Extent2D swap_chain_extent,
    vk::PipelineLayout pipeline_layout)
{
    vk::GraphicsPipelineCreateInfo pipeline_info{};

    auto shader_stages = create_shader_stages(device);
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    pipeline_info.pStages = shader_stages.data();

    auto input_assembly_state = input_assembly_state_info();
    pipeline_info.pInputAssemblyState = &input_assembly_state;

    auto vertex_input_state =
        vertex_input_state_info(binding_description, attribute_descriptions);
    pipeline_info.pVertexInputState = &vertex_input_state;

    auto viewport_state = viewport_state_info(swap_chain_extent);
    pipeline_info.pViewportState = &viewport_state.info;

    auto rasterization_state = rasterization_state_info();
    pipeline_info.pRasterizationState = &rasterization_state;

    auto multisample_state = multi_sample_state_info();
    pipeline_info.pMultisampleState = &multisample_state;

    auto depth_stencil_state = depth_stencil_state_info();
    pipeline_info.pDepthStencilState = &depth_stencil_state;

    auto color_blend_state = color_blend_state_info();
    pipeline_info.pColorBlendState = &color_blend_state.info;

    pipeline_info.pDynamicState = nullptr;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = framebuffer_set.render_pass();
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = nullptr;
    pipeline_info.basePipelineIndex = -1;

    vk::Pipeline pipeline = static_cast<vk::Device>(device).createGraphicsPipeline(
        nullptr, pipeline_info, nullptr);

    for (vk::PipelineShaderStageCreateInfo info : shader_stages) {
        static_cast<vk::Device>(device).destroyShaderModule(info.module);
    }

    return pipeline;
}

Quad::Quad(
    Device& device,
    // DescriptorManager& descriptor_manager,
    // TextureManager& texture_manager,
    SwapChain& swap_chain,
    Texture& depth_texture)
    : _device{device},
      _swap_chain{swap_chain},
      _clear_framebuffer_set{_device, _swap_chain, &depth_texture, {{"usage", "Clear"}}},
      _draw_framebuffer_set{_device, _swap_chain, nullptr, {{"usage", "Quad"}}},
      _command_buffers{create_command_buffers(_device, swap_chain)},
      _image_available_semaphore{static_cast<vk::Device>(_device).createSemaphore({})},
      _render_finished_semaphore{static_cast<vk::Device>(_device).createSemaphore({})},
      _vertex_buffer{create_vertex_buffer(_device)},
      _pipeline_layout{create_pipeline_layout(_device)},
      _pipeline{create_pipeline(_device, _draw_framebuffer_set, QuadVertex::binding_description(), QuadVertex::attribute_description(), swap_chain.extent(), _pipeline_layout)}
// _uniform_buffer{create_uniform_buffer(_device)}
// _uniform_buffer{create_uniform_buffer(_device)},
// _descriptor_manager{descriptor_manager},
// _descriptor_set{create_descriptor_set(_descriptor_manager, _uniform_buffer)},
// _pipeline{
//     _device,
//     descriptor_manager,
//     texture_manager,
//     swap_chain,
//     nullptr,
//     QuadVertex::binding_description(),
//     QuadVertex::attribute_description(),
//     nullptr,
//     _descriptor_set.layout(),
//     {{"vertexShader", "vert.spv"},
//      {"fragmentShader", "frag.spv"},
//      {"usage", "Quad"}}}
{
    std::cout << "Quad constructed\n";
}

Quad::~Quad()
{
    static_cast<vk::Device>(_device).destroySemaphore(_render_finished_semaphore);
    static_cast<vk::Device>(_device).destroySemaphore(_image_available_semaphore);
    static_cast<vk::Device>(_device).destroyPipeline(_pipeline);
    static_cast<vk::Device>(_device).destroyPipelineLayout(_pipeline_layout);
}

// void Quad::update_uniform_buffer()
// {
//     _uniform.world_view = glm::mat4(1.0f);
//     _uniform.proj = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f);
//     void* data = static_cast<vk::Device>(_device).mapMemory(
//         _uniform_buffer.memory(), 0, sizeof(_uniform), {});
//     memcpy(data, &_uniform, sizeof(QuadUniform));
//     static_cast<vk::Device>(_device).unmapMemory(_uniform_buffer.memory());
// }

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

static void draw_quad_pass(
    vk::CommandBuffer command_buffer,
    int framebuffer_index,
    vk::Extent2D swap_chain_extent,
    FramebufferSet& framebuffer_set,
    vk::Pipeline pipeline,
    Buffer& vertex_buffer)
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = framebuffer_set.render_pass();
    render_pass_info.framebuffer =
        framebuffer_set.frame_buffer(framebuffer_index);
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = swap_chain_extent;

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    command_buffer.bindVertexBuffers(0, {vertex_buffer}, {0});

    // command_buffer.bindDescriptorSets(
    //     vk::PipelineBindPoint::eGraphics,
    //     quad.pipeline().layout(),
    //     0,
    //     {quad.descriptor_set()},
    //     nullptr);

    // command_buffer.bindDescriptorSets(
    //     vk::PipelineBindPoint::eGraphics,
    //     quad.pipeline().layout(),
    //     1,
    //     {quad.pipeline().descriptor_set()},
    //     nullptr);

    command_buffer.draw(6, 1, 0, 0);

    command_buffer.endRenderPass();
}

void Quad::draw_frame()
{
    uint32_t image_index = 0;
    static_cast<vk::Device>(_device).acquireNextImageKHR(
        _swap_chain,
        std::numeric_limits<uint64_t>::max(),
        _image_available_semaphore,
        nullptr,
        &image_index);

    vk::CommandBuffer command_buffer = _command_buffers[image_index];

    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    command_buffer.begin(begin_info);

    clear_pass(
        command_buffer,
        _clear_framebuffer_set.render_pass(),
        _clear_framebuffer_set.frame_buffer(image_index),
        _swap_chain.extent());

    draw_quad_pass(command_buffer, image_index, _swap_chain.extent(), _draw_framebuffer_set, _pipeline, _vertex_buffer);

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
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    _device.present_queue().presentKHR(present_info);
    static_cast<vk::Device>(_device).waitIdle();
}
