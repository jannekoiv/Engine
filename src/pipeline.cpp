
#include "pipeline.h"
#include "base.h"
#include "descriptor_manager.h"
#include "device.h"
#include "swap_chain.h"
#include "texture_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vulkan/vulkan.hpp>

Pipeline::Pipeline(Pipeline&& rhs)
    : _device{rhs._device},
      _framebuffer_set{std::move(rhs._framebuffer_set)},
      _texture{rhs._texture},
      _descriptor_set{std::move(rhs._descriptor_set)},
      _pipeline_layout{rhs._pipeline_layout},
      _pipeline{rhs._pipeline}
{
    rhs._pipeline_layout = nullptr;
    rhs._pipeline = nullptr;
    rhs._texture = nullptr;
}

bool has_key(const nlohmann::json& json, const std::string& key)
{
    if (json.find(key) != json.end()) {
        return true;
    } else {
        return false;
    }
}

nlohmann::json read_reflection_file(std::string filename)
{
    std::ifstream reflection_file{filename};

    if (!reflection_file.is_open()) {
        throw std::runtime_error("Failed to open reflection file!");
    }
    nlohmann::json reflection;
    reflection_file >> reflection;
    return reflection;
}

std::string reflection_filename(std::string shader_filename)
{
    auto n = shader_filename.substr(0, shader_filename.find(".", 0)) + ".json";
    std::cout << n << "\n";
    return n;
}

//std::vector<vk::DescriptorSetLayoutBinding> bindings = {
//    {0,
//     vk::DescriptorType::eUniformBuffer,
//     1,
//     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}};

struct LayoutBinding {
    LayoutBinding(
        size_t set,
        uint32_t binding,
        vk::DescriptorType descriptor_type,
        uint32_t descriptor_count,
        vk::ShaderStageFlags stage_flags)
    {
    }
    size_t set;
    vk::DescriptorSetLayoutBinding binding;
};

std::list<LayoutBinding> export_layout_bindings(const nlohmann::json& reflection)
{

    std::list<LayoutBinding> layout_bindings;


    if(has_key(reflection, "ubos")) {
        for (auto& ubo : reflection["ubos"]) {
            layout_bindings.emplace_back(
                ubo["set"],
                ubo["binding"],
                vk::DescriptorType::eUniformBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex);
        }
    }

    //for (auto& texture : reflection["textures"]) {
    //    layoutBindings.emplace_back(
    //        texture["set"],
    //        texture["binding"],
    //        vk::DescriptorType::eUniformBuffer,
    //        1,
    //        vk::ShaderStageFlagBits::eVertex);
    //}

    return layout_bindings;
}

vk::PipelineLayout create_pipeline_layout(
    Device& device,
    vk::DescriptorSetLayout descriptor_set_layout1,
    vk::DescriptorSetLayout descriptor_set_layout2,
    vk::DescriptorSetLayout descriptor_set_layout3,
    const nlohmann::json& json)
{
    std::list<LayoutBinding> vertex_shader_bindings;
    //if (hasKey(json, "vertexShader")) {
    //    exportLayoutBindings(
    //        readReflectionFile(reflectionFilename(json["vertexShader"])));
    //}

    // if (has_key(json, "vertexShader")) {
    //     auto refl = read_reflection_file(reflection_filename(json["vertexShader"]));

    //     export_layout_bindings(refl);
    // }

    //std::list<LayoutBinding> fragmentShaderBindings;
    //if (hasKey(json, "fragmentShader")) {
    //    fragmentShaderBindings = exportLayoutBindings(json["fragmentShader"]);
    //}

    vk::PushConstantRange push_constant_range{};
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(float) * 16;
    push_constant_range.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::PipelineLayoutCreateInfo info{};

    std::vector<vk::DescriptorSetLayout> layouts{};
    if (descriptor_set_layout1) {
        layouts.push_back(descriptor_set_layout1);
    }
    if (descriptor_set_layout2) {
        layouts.push_back(descriptor_set_layout2);
    }
    if (descriptor_set_layout3) {
        layouts.push_back(descriptor_set_layout3);
    }

    info.setLayoutCount = static_cast<uint32_t>(layouts.size());
    info.pSetLayouts = layouts.data();

    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &push_constant_range;

    vk::PipelineLayout layout =
        static_cast<vk::Device>(device).createPipelineLayout(info);
    return layout;
}

vk::ShaderModule create_shader_from_file(vk::Device device, std::string filename)
{
    auto code = read_file(filename);
    vk::ShaderModuleCreateInfo info{};
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    vk::ShaderModule module = device.createShaderModule(info, nullptr);
    return module;
}

vk::PipelineRasterizationStateCreateInfo rasterization_state_info(
    const nlohmann::json& json)
{
    vk::PipelineRasterizationStateCreateInfo info{};

    if (has_key(json, "polygonMode")) {
        auto mode = json["polygonMode"];
        if (mode == "Fill") {
            info.polygonMode = vk::PolygonMode::eFill;
        } else if (mode == "Line") {
            info.polygonMode = vk::PolygonMode::eLine;
        } else if (mode == "Point") {
            info.polygonMode = vk::PolygonMode::ePoint;
        }
    }
    info.lineWidth = 1.0f;

    if (has_key(json, "cullMode")) {
        auto mode = json["cullMode"];
        if (mode == "None") {
            info.cullMode = vk::CullModeFlagBits::eNone;
        } else if (mode == "Front") {
            info.cullMode = vk::CullModeFlagBits::eFront;
        } else if (mode == "Back") {
            info.cullMode = vk::CullModeFlagBits::eBack;
        } else if (mode == "FrontAndBack") {
            info.cullMode = vk::CullModeFlagBits::eFrontAndBack;
        }
    }
    info.frontFace = vk::FrontFace::eCounterClockwise;

    return info;
}

vk::CompareOp compare_op(const std::string& op)
{
    if (op == "Never") {
        return vk::CompareOp::eNever;
    } else if (op == "Less") {
        return vk::CompareOp::eLess;
    } else if (op == "Equal") {
        return vk::CompareOp::eEqual;
    } else if (op == "LessOrEqual") {
        return vk::CompareOp::eLessOrEqual;
    } else if (op == "Greater") {
        return vk::CompareOp::eGreater;
    } else if (op == "NotEqual") {
        return vk::CompareOp::eNotEqual;
    } else if (op == "GreaterOrEqual") {
        return vk::CompareOp::eGreaterOrEqual;
    } else if (op == "Always") {
        return vk::CompareOp::eAlways;
    } else {
        throw std::runtime_error{"Invalid comparison operator.\n"};
    }
}

vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_info(
    const nlohmann::json& json)
{
    vk::PipelineDepthStencilStateCreateInfo info{};

    if (has_key(json, "depthTestEnable")) {
        info.depthTestEnable = json["depthTestEnable"];
    }
    if (has_key(json, "depthWriteEnable")) {
        info.depthWriteEnable = json["depthWriteEnable"];
    }
    if (has_key(json, "depthCompareOp")) {
        info.depthCompareOp = compare_op(json["depthCompareOp"]);
    }

    return info;
}

struct ColorBlendStateCreateInfo {
    std::vector<vk::PipelineColorBlendAttachmentState> attachments;
    vk::PipelineColorBlendStateCreateInfo info;
};

ColorBlendStateCreateInfo color_blend_state_info(const nlohmann::json& json)
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

vk::PipelineMultisampleStateCreateInfo multi_sample_state_info(
    const nlohmann::json& json)
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

struct ViewportStateCreateInfo {
    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
    vk::PipelineViewportStateCreateInfo info;
};

ViewportStateCreateInfo viewport_state_info(
    const nlohmann::json& json, vk::Extent2D swap_chain_extent)
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

vk::PipelineVertexInputStateCreateInfo vertex_input_state_info(
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

vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_info()
{
    vk::PipelineInputAssemblyStateCreateInfo info{};
    info.topology = vk::PrimitiveTopology::eTriangleList;
    info.primitiveRestartEnable = false;
    return info;
}

std::vector<vk::PipelineShaderStageCreateInfo> create_shader_stages(
    Device& device, const nlohmann::json& json)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};

    if (has_key(json, "vertexShader")) {
        vk::PipelineShaderStageCreateInfo vertex_shader_stage_info;
        vertex_shader_stage_info.stage = vk::ShaderStageFlagBits::eVertex;
        vertex_shader_stage_info.module = create_shader_from_file(device, json["vertexShader"]);
        vertex_shader_stage_info.pName = "main";
        shader_stages.push_back(vertex_shader_stage_info);
    }

    if (has_key(json, "fragmentShader")) {
        vk::PipelineShaderStageCreateInfo fragment_shader_stage_info;
        fragment_shader_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
        fragment_shader_stage_info.module = create_shader_from_file(device, json["fragmentShader"]);
        fragment_shader_stage_info.pName = "main";
        shader_stages.push_back(fragment_shader_stage_info);
    }

    return shader_stages;
}

vk::Pipeline create_pipeline(
    Device& device,
    FramebufferSet& framebuffer_set,
    vk::VertexInputBindingDescription binding_description,
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions,
    vk::Extent2D swap_chain_extent,
    vk::PipelineLayout pipeline_layout,
    const nlohmann::json& json)
{
    vk::GraphicsPipelineCreateInfo pipeline_info{};

    auto shader_stages = create_shader_stages(device, json);
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    pipeline_info.pStages = shader_stages.data();

    auto input_assembly_state = input_assembly_state_info();
    pipeline_info.pInputAssemblyState = &input_assembly_state;

    auto vertex_input_state =
        vertex_input_state_info(binding_description, attribute_descriptions);
    pipeline_info.pVertexInputState = &vertex_input_state;

    auto viewport_state = viewport_state_info(json, swap_chain_extent);
    pipeline_info.pViewportState = &viewport_state.info;

    auto rasterization_state = rasterization_state_info(json);
    pipeline_info.pRasterizationState = &rasterization_state;

    auto multisample_state = multi_sample_state_info(json);
    pipeline_info.pMultisampleState = &multisample_state;

    auto depth_stencil_state = depth_stencil_state_info(json);
    pipeline_info.pDepthStencilState = &depth_stencil_state;

    auto color_blend_state = color_blend_state_info(json);
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

static DescriptorSet create_descriptor_set(
    DescriptorManager& descriptor_manager, Texture* texture)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0,
         vk::DescriptorType::eCombinedImageSampler,
         1,
         vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptor_set = descriptor_manager.create_descriptor_set(bindings);

    if (texture) {
        vk::DescriptorImageInfo image_info{};
        image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        image_info.imageView = texture->image_view();
        image_info.sampler = texture->sampler();
        descriptor_set.write_descriptors({{0, 0, 1, &image_info}});
    }

    return descriptor_set;
}

static Texture* create_texture_from_file(
    TextureManager& texture_manager, const nlohmann::json& json)
{
    if (has_key(json, "texture")) {
        return &texture_manager.create_texture_from_file(
            json["texture"], vk::SamplerAddressMode::eRepeat);
    } else {
        return nullptr;
    }
}

Pipeline::Pipeline(
    Device& device,
    DescriptorManager& descriptor_manager,
    TextureManager& texture_manager,
    SwapChain& swap_chain,
    Texture* depth_texture,
    vk::VertexInputBindingDescription binding_description,
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions,
    vk::DescriptorSetLayout descriptor_set_layout1,
    vk::DescriptorSetLayout descriptor_set_layout2,
    const nlohmann::json& json)
    : _device{device},
      _framebuffer_set{_device, swap_chain, depth_texture, json},
      _texture{create_texture_from_file(texture_manager, json)},
      _descriptor_set{create_descriptor_set(descriptor_manager, _texture)},
      _pipeline_layout{create_pipeline_layout(
          _device,
          descriptor_set_layout1,
          _descriptor_set.layout(),
          descriptor_set_layout2,
          json)},
      _pipeline{create_pipeline(
          _device,
          _framebuffer_set,
          binding_description,
          attribute_descriptions,
          swap_chain.extent(),
          _pipeline_layout,
          json)}
{
}

Pipeline::~Pipeline()
{
    static_cast<vk::Device>(_device).destroyPipeline(_pipeline);
    static_cast<vk::Device>(_device).destroyPipelineLayout(_pipeline_layout);
}
