#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#define GENERATOR_USE_GLM
#include "includes.h"
#include <generator/generator.hpp>
#include <ofbx.h>
#include <stb/stb_image.h>

Object::Object(Object&& rhs)
    : _context{rhs._context},
      _swap_chain{rhs._swap_chain},
      _render_pass{rhs._render_pass},
      _framebuffers{std::move(rhs._framebuffers)},
      _command_buffers{std::move(rhs._command_buffers)},
      _vertex_buffer{std::move(rhs._vertex_buffer)},
      _index_buffer{std::move(rhs._index_buffer)},
      _uniform_buffer{std::move(rhs._uniform_buffer)},
      _texture{std::move(rhs._texture)},
      _descriptor_pool{rhs._descriptor_pool},
      _descriptor_set_layout{rhs._descriptor_set_layout},
      _descriptor_set{rhs._descriptor_set},
      _pipeline_layout{rhs._pipeline_layout},
      _pipeline{rhs._pipeline},
      _world_matrix{rhs._world_matrix},
      _wireframe{rhs._wireframe}
{
    rhs._render_pass = nullptr;
    rhs._descriptor_pool = nullptr;
    rhs._descriptor_set_layout = nullptr;
    rhs._descriptor_set = nullptr;
    rhs._pipeline_layout = nullptr;
    rhs._pipeline = nullptr;
}

Object::~Object()
{
    static_cast<vk::Device>(_context.device()).destroyRenderPass(_render_pass);
    for (auto framebuffer : _framebuffers) {
        static_cast<vk::Device>(_context.device()).destroyFramebuffer(framebuffer);
    }
    static_cast<vk::Device>(_context.device()).destroyPipeline(_pipeline);
    static_cast<vk::Device>(_context.device()).destroyPipelineLayout(_pipeline_layout);
    static_cast<vk::Device>(_context.device()).destroyDescriptorPool(_descriptor_pool);
    static_cast<vk::Device>(_context.device())
        .destroyDescriptorSetLayout(_descriptor_set_layout);
}

vk::VertexInputBindingDescription MeshVertex::binding_description()
{
    vk::VertexInputBindingDescription desc = {};
    desc.binding = 0;
    desc.stride = sizeof(MeshVertex);
    desc.inputRate = vk::VertexInputRate::eVertex;
    return desc;
}

std::vector<vk::VertexInputAttributeDescription> MeshVertex::attribute_description()
{
    std::vector<vk::VertexInputAttributeDescription> desc{};

    desc.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(MeshVertex, position));
    desc.emplace_back(
        1, 0, vk::Format::eA8B8G8R8UnormPack32, offsetof(MeshVertex, color));
    desc.emplace_back(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(MeshVertex, normal));
    desc.emplace_back(3, 0, vk::Format::eR32G32Sfloat, offsetof(MeshVertex, texcoord));

    return desc;
}

void Object::copy_vertices(const std::vector<MeshVertex>& vertices)
{
    vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

    Buffer staging_buffer(
        _context.device(),
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    staging_buffer.unmap_memory();
    staging_buffer.copy(_context.command_pool(), _vertex_buffer);
}

static Buffer create_vertex_buffer(
    Context& context, const std::vector<MeshVertex>& vertices)
{
    vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

    Buffer staging_buffer(
        context.device(),
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    staging_buffer.unmap_memory();

    Buffer vertex_buffer(
        context.device(),
        size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    staging_buffer.copy(context.command_pool(), vertex_buffer);

    return vertex_buffer;
}

static Buffer create_index_buffer(Context& context, const std::vector<uint32_t> indices)
{
    vk::DeviceSize size = sizeof(indices[0]) * indices.size();

    Buffer staging_buffer(
        context.device(),
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, indices.data(), static_cast<size_t>(size));
    staging_buffer.unmap_memory();

    Buffer index_buffer(
        context.device(),
        size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    staging_buffer.copy(context.command_pool(), index_buffer);

    return index_buffer;
}

static Buffer create_uniform_buffer(Device& device)
{
    return Buffer(
        device,
        sizeof(glm::mat4) + sizeof(SceneUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
}

vk::DescriptorPool create_descriptor_pool(vk::Device device)
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

vk::DescriptorSetLayout create_descriptor_set_layout(vk::Device device)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
        {1,
         vk::DescriptorType::eCombinedImageSampler,
         1,
         vk::ShaderStageFlagBits::eFragment}};

    vk::DescriptorSetLayoutCreateInfo info;
    info.bindingCount = static_cast<uint32_t>(bindings.size());
    info.pBindings = bindings.data();

    vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(info, nullptr);
    return layout;
}

vk::DescriptorSet create_descriptor_set(
    vk::Device device,
    vk::DescriptorPool pool,
    vk::DescriptorSetLayout layout,
    Buffer& uniform_buffer,
    Texture& texture)
{
    vk::DescriptorSetAllocateInfo info;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;

    vk::DescriptorSet set = device.allocateDescriptorSets(info).front();

    vk::DescriptorBufferInfo buffer_info{
        uniform_buffer,
        buffer_info.offset = 0,
        buffer_info.range = sizeof(glm::mat4) + sizeof(SceneUniform)};

    vk::DescriptorImageInfo image_info{
        texture.sampler(), texture.image_view(), vk::ImageLayout::eShaderReadOnlyOptimal};

    std::vector<vk::WriteDescriptorSet> writes{
        {set,
         0,
         0,
         1,
         vk::DescriptorType::eUniformBuffer,
         nullptr,
         &buffer_info,
         nullptr},
        {set,
         1,
         0,
         1,
         vk::DescriptorType::eCombinedImageSampler,
         &image_info,
         nullptr,
         nullptr}};
    device.updateDescriptorSets(writes, nullptr);

    return set;
}

static vk::PipelineLayout create_pipeline_layout(
    Device& device, vk::DescriptorSetLayout descriptor_set_layout)
{
    vk::PipelineLayoutCreateInfo info{};
    info.setLayoutCount = 1;
    info.pSetLayouts = &descriptor_set_layout;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;

    vk::PipelineLayout layout =
        static_cast<vk::Device>(device).createPipelineLayout(info);
    return layout;
}

static vk::ShaderModule create_vertex_shader(vk::Device device)
{
    const char glsl_code[] = R"(
	#version 450
	#extension GL_ARB_separate_shader_objects : enable
        
        layout(binding = 0) uniform UniformBufferObject {
	    mat4 world;
	    mat4 view;
	    mat4 proj;
	    vec3 light_dir;
        } ubo;

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec4 in_color;
	layout(location = 2) in vec3 in_normal;
	layout(location = 3) in vec2 in_texcoord;

	layout(location = 0) out vec3 out_color;
	layout(location = 1) out vec2 out_texcoord;

	void main()
	{
	    gl_Position = ubo.proj * ubo.view * ubo.world * vec4(in_position, 1.0);
	    out_color = in_color.rgb;
	    out_texcoord = in_texcoord;
	}
    )";

    ShaderCompiler compiler{device};
    return compiler.compile(vk::ShaderStageFlagBits::eVertex, glsl_code);
}

static vk::ShaderModule create_fragment_shader(vk::Device device)
{
    const char glsl_code[] = R"(
        #version 450
        #extension GL_ARB_separate_shader_objects : enable

	layout(binding = 1) uniform sampler2D tex_sampler;

	layout(location = 0) in vec3 in_color;
	layout(location = 1) in vec2 in_texcoord;

	layout(location = 0) out vec4 out_color;

	void main()
	{
	    out_color = vec4(in_color, 1.0);// * texture(tex_sampler, in_texcoord);
	}
    )";

    ShaderCompiler compiler{device};
    return compiler.compile(vk::ShaderStageFlagBits::eFragment, glsl_code);
}

static std::vector<vk::PipelineShaderStageCreateInfo> create_shader_stages(Device& device)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};

    vk::PipelineShaderStageCreateInfo vertex_shader_stage;
    vertex_shader_stage.stage = vk::ShaderStageFlagBits::eVertex;
    vertex_shader_stage.module = create_vertex_shader(device);
    vertex_shader_stage.pName = "main";
    shader_stages.push_back(vertex_shader_stage);

    vk::PipelineShaderStageCreateInfo fragment_shader_stage;
    fragment_shader_stage.stage = vk::ShaderStageFlagBits::eFragment;
    fragment_shader_stage.module = create_fragment_shader(device);
    fragment_shader_stage.pName = "main";
    shader_stages.push_back(fragment_shader_stage);

    return shader_stages;
}

static vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_info(bool wireframe)
{
    vk::PipelineInputAssemblyStateCreateInfo info{};
    if (wireframe) {
        info.topology = vk::PrimitiveTopology::eLineList;
    } else {
        info.topology = vk::PrimitiveTopology::eTriangleList;
    }
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

static vk::PipelineRasterizationStateCreateInfo rasterization_state_info(bool wireframe)
{
    vk::PipelineRasterizationStateCreateInfo info{};

    info.polygonMode = vk::PolygonMode::eFill;
    info.lineWidth = 1.0f;
    info.cullMode = vk::CullModeFlagBits::eFront;
    info.frontFace = vk::FrontFace::eCounterClockwise;
    if (wireframe) {
        info.polygonMode = vk::PolygonMode::eLine;
        info.cullMode = vk::CullModeFlagBits::eNone;
    }

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
    info.depthCompareOp = vk::CompareOp::eLess;
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
    vk::RenderPass render_pass,
    vk::VertexInputBindingDescription binding_description,
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions,
    vk::Extent2D swap_chain_extent,
    vk::PipelineLayout pipeline_layout,
    bool wireframe)
{
    vk::GraphicsPipelineCreateInfo pipeline_info{};

    auto shader_stages = create_shader_stages(device);
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    pipeline_info.pStages = shader_stages.data();

    auto input_assembly_state = input_assembly_state_info(wireframe);
    pipeline_info.pInputAssemblyState = &input_assembly_state;

    auto vertex_input_state =
        vertex_input_state_info(binding_description, attribute_descriptions);
    pipeline_info.pVertexInputState = &vertex_input_state;

    auto viewport_state = viewport_state_info(swap_chain_extent);
    pipeline_info.pViewportState = &viewport_state.info;

    auto rasterization_state = rasterization_state_info(wireframe);
    pipeline_info.pRasterizationState = &rasterization_state;

    auto multisample_state = multi_sample_state_info();
    pipeline_info.pMultisampleState = &multisample_state;

    auto depth_stencil_state = depth_stencil_state_info();
    pipeline_info.pDepthStencilState = &depth_stencil_state;

    auto color_blend_state = color_blend_state_info();
    pipeline_info.pColorBlendState = &color_blend_state.info;

    pipeline_info.pDynamicState = nullptr;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = nullptr;
    pipeline_info.basePipelineIndex = -1;

    vk::Pipeline pipeline = static_cast<vk::Device>(device)
                                .createGraphicsPipeline(nullptr, pipeline_info, nullptr)
                                .value;

    for (vk::PipelineShaderStageCreateInfo info : shader_stages) {
        static_cast<vk::Device>(device).destroyShaderModule(info.module);
    }

    return pipeline;
}

vk::RenderPass create_render_pass(Device& device, vk::Format swap_chain_format)
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

std::vector<vk::Framebuffer> create_framebuffers(
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

Texture create_texture_from_pixels(
    Context& context,
    int width,
    int height,
    stbi_uc* pixels,
    vk::SamplerAddressMode address_mode)
{
    if (!pixels) {
        throw std::runtime_error("Failed to create texture image!");
    }

    const int bytes_per_pixel = 4;
    vk::DeviceSize image_size = width * height * bytes_per_pixel;

    Buffer staging_buffer(
        context.device(),
        image_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, pixels, image_size);
    staging_buffer.unmap_memory();

    Texture texture{
        context.device(),
        vk::ImageViewType::e2D,
        1,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        address_mode};

    texture.transition_layout(
        context.command_pool(),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);
    staging_buffer.copy_to_texture(context.command_pool(), texture, 0);
    texture.transition_layout(
        context.command_pool(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    return texture;
}

Texture create_texture_from_file(
    Context& context, std::string filename, vk::SamplerAddressMode address_mode)
{
    if (filename.empty()) {
        throw std::runtime_error("Failed to load texture image!");
    }

    int width = 0;
    int height = 0;
    int channel_count = 0;

    stbi_uc* pixels =
        stbi_load(filename.data(), &width, &height, &channel_count, STBI_rgb_alpha);

    auto texture =
        create_texture_from_pixels(context, width, height, pixels, address_mode);
    stbi_image_free(pixels);
    return texture;
}

Texture create_dummy_texture(Context& context, vk::SamplerAddressMode address_mode)
{
    int width = 1;
    int height = 1;
    stbi_uc pixels[] = {0xff, 0xff, 0xff, 0xff};
    return create_texture_from_pixels(context, width, height, pixels, address_mode);
}

Object::Object(Context& context, const MeshGeometry& geometry, bool wireframe)
    : _context{context},
      _swap_chain{_context.swap_chain()},
      _render_pass{create_render_pass(_context.device(), _context.swap_chain().format())},
      _framebuffers{create_framebuffers(
          _context.device(),
          _context.swap_chain(),
          _context.depth_texture(),
          _render_pass)},
      _vertex_buffer{create_vertex_buffer(_context, geometry.vertices)},
      _index_buffer{create_index_buffer(_context, geometry.indices)},
      _uniform_buffer{create_uniform_buffer(_context.device())},
      // _texture{create_texture_from_file(
      //     context, "brick.png", vk::SamplerAddressMode::eRepeat)},
      _texture{create_dummy_texture(context, vk::SamplerAddressMode::eRepeat)},
      _descriptor_pool{create_descriptor_pool(_context.device())},
      _descriptor_set_layout{create_descriptor_set_layout(_context.device())},
      _descriptor_set{create_descriptor_set(
          _context.device(),
          _descriptor_pool,
          _descriptor_set_layout,
          _uniform_buffer,
          _texture)},
      _pipeline_layout{create_pipeline_layout(_context.device(), _descriptor_set_layout)},
      _pipeline{create_pipeline(
          _context.device(),
          _render_pass,
          MeshVertex::binding_description(),
          MeshVertex::attribute_description(),
          context.swap_chain().extent(),
          _pipeline_layout,
          wireframe)},
      _world_matrix{1.0f},
      _wireframe{wireframe}
{
    std::cout << "Object constructed\n";
}

size_t file_size(std::ifstream& file)
{
    const auto begin = file.tellg();
    file.seekg(0, std::ios::end);
    const auto end = file.tellg();
    file.seekg(0, std::ios::beg);
    return end - begin;
}

ofbx::IScene* read_scene(std::ifstream& file)
{
    auto* buffer = file.rdbuf();

    std::vector<char> data(file_size(file));
    buffer->sgetn(data.data(), data.size());

    ofbx::IScene* scene = ofbx::load(
        (ofbx::u8*)data.data(), data.size(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
    return scene;
}

glm::vec3 ofbx_to_glm(ofbx::Vec3 vec)
{
    return glm::vec3{vec.x, vec.z, -vec.y};
}

glm::vec2 ofbx_to_glm(ofbx::Vec2 vec)
{
    return glm::vec2{vec.x, 1.0f - vec.y};
}

uint32_t packed_color(ofbx::Vec4 vec)
{
    uint32_t color = int((vec.x) * 255) << 0;
    color |= int((vec.y) * 255) << 8;
    color |= int((vec.z) * 255) << 16;
    color |= int((vec.w) * 255) << 24;
    return color;
}

uint32_t packed_color(ofbx::Vec3 vec)
{
    uint32_t color = int((vec.x + 1.0f) * 100) << 0;
    color |= int((vec.y + 1.0f) * 100) << 8;
    color |= int((vec.z + 1.0f) * 100) << 16;
    return color;
}

uint32_t packed_color(glm::vec3 vec)
{
    uint32_t color = int((vec.x + 1.0f) * 100) << 0;
    color |= int((vec.y + 1.0f) * 100) << 8;
    color |= int((vec.z + 1.0f) * 100) << 16;
    return color;
}

uint32_t packed_color(ofbx::Vec2 vec)
{
    uint32_t color = int((vec.x + 1.0f) * 100) << 0;
    color |= int((vec.y + 1.0f) * 100) << 8;
    return color;
}

uint32_t convert_index(int index)
{
    if (index < 0) {
        return -index - 1;
    } else {
        return index;
    }
}

const std::vector<MeshVertex> export_vertices(const ofbx::Geometry& geom)
{
    std::cout << "exporting\n";
    std::vector<MeshVertex> vertices(geom.getVertexCount());
    if (geom.getVertices() && geom.getNormals() && geom.getUVs(0)) {
        auto* colors = geom.getColors();
        for (auto i = 0; i < vertices.size(); i++) {
            vertices[i].position = ofbx_to_glm(geom.getVertices()[i]);
            vertices[i].normal = ofbx_to_glm(geom.getNormals()[i]);
            vertices[i].texcoord = ofbx_to_glm(geom.getUVs(0)[i]);
            if (colors) {
                vertices[i].color = packed_color(colors[i]);
            }
        }
    } else {
        std::cout << "No vertex data to export.\n";
    }
    return vertices;
}

const std::vector<uint32_t> export_indices(const ofbx::Geometry& geom)
{
    std::vector<uint32_t> indices(geom.getIndexCount());
    for (auto i = 0; i < indices.size(); i++) {
        indices[i] = convert_index(geom.getFaceIndices()[i]);
    }
    return indices;
}

const std::string export_texture_filename(const ofbx::Mesh& mesh)
{
    char filename[128] = {};
    const ofbx::Material* material = mesh.getMaterial(0);
    if (material) {
        const ofbx::Texture* texture =
            material->getTexture(ofbx::Texture::TextureType::DIFFUSE);
        if (texture) {
            texture->getRelativeFileName().toString(filename);
        }
    }
    std::cout << "filename is " << filename << "\n";
    return filename;
}

void update_uniform_buffer(
    vk::Device device,
    const SceneUniform& uniform,
    const glm::mat4& world_matrix,
    Buffer& uniform_buffer)
{
    char* data = static_cast<char*>(uniform_buffer.map_memory());
    memcpy(data, &world_matrix, sizeof(glm::mat4));
    memcpy(data + sizeof(glm::mat4), &uniform, sizeof(SceneUniform));
    uniform_buffer.unmap_memory();
}

void Object::draw(Context& context, SceneUniform& uniform)
{
    update_uniform_buffer(context.device(), uniform, _world_matrix, _uniform_buffer);

    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = _render_pass;
    render_pass_info.framebuffer = _framebuffers[context.image_index()];
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = context.swap_chain().extent();

    auto command_buffer = context.command_buffer();

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
    command_buffer.bindVertexBuffers(0, {_vertex_buffer}, {0});
    command_buffer.bindIndexBuffer(_index_buffer, 0, vk::IndexType::eUint32);

    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _pipeline_layout, 0, _descriptor_set, nullptr);

    if (_wireframe) {
        command_buffer.draw(_index_buffer.size() / 4, 1, 0, 0);
    } else {
        command_buffer.drawIndexed(
            static_cast<uint32_t>(_index_buffer.size() / 4), 1, 0, 0, 0);
    }

    command_buffer.endRenderPass();
}

MeshGeometry create_geometry_from_file(std::string filename)
{
    std::ifstream file{filename, std::ifstream::binary};
    if (!file.is_open()) {
        std::cout << "No fbx file\n";
    }

    auto scene = read_scene(file);
    const ofbx::Mesh& mesh = *scene->getMesh(0);
    const ofbx::Geometry& geom = *mesh.getGeometry();

    return MeshGeometry{
        std::move(export_vertices(geom)), std::move(export_indices(geom))};
}

Object create_object_from_file(Context& context, std::string filename)
{
    return Object{context, create_geometry_from_file(filename), false};
}
