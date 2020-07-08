
#include "../Include/Pipeline.h"
#include "../Include/Base.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Device.h"
#include "../Include/SwapChain.h"
#include "../Include/TextureManager.h"
#include <fstream>
#include <iostream>
#include <spirv_cross.hpp>
#include <sstream>
#include <vulkan/vulkan.hpp>

Pipeline::Pipeline(Pipeline&& rhs)
    : mDevice{rhs.mDevice},
      mFramebufferSet{std::move(rhs.mFramebufferSet)},
      mTexture{rhs.mTexture},
      mDescriptorSet{std::move(rhs.mDescriptorSet)},
      mPipelineLayout{rhs.mPipelineLayout},
      mPipeline{rhs.mPipeline}
{
    rhs.mPipelineLayout = nullptr;
    rhs.mPipeline = nullptr;
    rhs.mTexture = nullptr;
}

std::vector<uint32_t> readSpirvFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    std::vector<uint32_t> buffer32(buffer.size() / sizeof(uint32_t));
    memcpy(buffer32.data(), buffer.data(), buffer.size());
    return buffer32;
}

bool hasKey(const nlohmann::json& json, const std::string& key)
{
    if (json.find(key) != json.end()) {
        return true;
    } else {
        return false;
    }
}

struct UniformMember {
    std::string name;
    spirv_cross::SPIRType type;
};

struct Uniform {
    std::string name;
    unsigned set;
    unsigned binding;
    std::list<UniformMember> members;
};

struct Sampler {
    std::string name;
    unsigned set;
    unsigned binding;
};

struct DescriptorInfo {
    std::list<Uniform> uniforms;
    //std::list<Sampler> samplers;
};

//std::vector<vk::DescriptorSetLayoutBinding> bindings = {
//    {0,
//     vk::DescriptorType::eUniformBuffer,
//     1,
//     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}};

std::string stringType(spirv_cross::SPIRType type)
{
    if (type.basetype == spirv_cross::SPIRType::Float) {
        return "float";
    } else if (type.basetype == spirv_cross::SPIRType::Double) {
        return "double";
    } else {
        return "unknown";
    }
}

std::vector<vk::DescriptorSetLayoutBinding> descriptorBindings(
    DescriptorInfo& descriptorInfo)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    for (auto& uniform : descriptorInfo.uniforms) {
        vk::DescriptorSetLayoutBinding binding;

        std::cout << uniform.name << " " << uniform.set << " " << uniform.binding << "\n";

        std::cout << "Uniform member count " << uniform.members.size() << "\n";
        for (auto& member : uniform.members) {
            std::cout << member.name << " " << stringType(member.type) << "\n";
        }

        std::cout << "\n";
    }

    return bindings;
}

vk::DescriptorSetLayout createDescriptorSetLayout(
    vk::Device device, DescriptorInfo& descriptorInfo)
{
    auto bindings = descriptorBindings(descriptorInfo);

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    //vk::DescriptorSetLayout layout =
    //    device.createDescriptorSetLayout(layoutInfo, nullptr);
    //return layout;
    return nullptr;
}

void printDescriptor(DescriptorInfo& descriptor)
{
    std::cout << "Uniform count " << descriptor.uniforms.size() << "\n\n";

    for (auto& uniform : descriptor.uniforms) {
        std::cout << uniform.name << " " << uniform.set << " " << uniform.binding << "\n";

        std::cout << "Uniform member count " << uniform.members.size() << "\n";
        for (auto& member : uniform.members) {
            std::cout << member.name << "\n";
        }

        std::cout << "\n";
    }

    //std::cout << "Sampler count " << descriptor.samplers.size() << "\n\n";

    //for (auto& sampler : descriptor.samplers) {
    //    std::cout << sampler.name << " " << sampler.set << " " << sampler.binding << "\n";
    //    std::cout << "\n";
    //}
}

DescriptorInfo parse(const nlohmann::json& json)
{
    DescriptorInfo descriptor;

    if (!hasKey(json, "fragmentShader")) {
        return descriptor;
    }

    std::cout << "Fragment shader resources:\n";
    auto spirvBinary = readSpirvFile(json["fragmentShader"]);
    spirv_cross::Compiler compiler(std::move(spirvBinary));

    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    for (auto& buffer : resources.uniform_buffers) {
        Uniform uniform{
            buffer.name,
            compiler.get_decoration(buffer.id, spv::DecorationDescriptorSet),
            compiler.get_decoration(buffer.id, spv::DecorationBinding)};

        auto& type = compiler.get_type(buffer.base_type_id);
        auto count = type.member_types.size();

        for (auto i = 0; i < count; i++) {
            UniformMember uniformMember{
                compiler.get_member_name(type.self, i),
                compiler.get_type(type.member_types[i])};
            uniform.members.push_back(uniformMember);

            if (compiler.get_member_name(type.self, i) == "vektori3") {
                std::cout << "COLUMNS " << compiler.get_type(type.member_types[i]).columns
                          << "\n";
                std::cout << "VECSIZE " << compiler.get_type(type.member_types[i]).vecsize
                          << "\n";
                std::cout << "WIDTH " << compiler.get_type(type.member_types[i]).width
                          << "\n";
            }

            auto memberType = compiler.get_type(type.member_types[i]);

            //std::cout << "      Uniform member type: " << memberType.basetype
            //          << " is float: "
            //          << (memberType.basetype == spirv_cross::SPIRType::Float) << "\n";

            //auto offset = compiler.type_struct_member_offset(type, i);
            //std::cout << "      Uniform member offset: " << offset << "\n";

            //auto memberSize = compiler.get_declared_struct_member_size(type, i);
            //std::cout << "      Uniform member size: " << memberSize << "\n";

            if (!memberType.array.empty()) {
                //auto arrayStride = compiler.type_struct_member_array_stride(type, i);
                //std::cout << "      Uniform member is array with stride: " << arrayStride
                //<< "\n";
            } else {
                //std::cout << "      Uniform member is not array\n";
            }

            //if (memberType.columns > 1) {
            //    auto matrixStride = compiler.type_struct_member_matrix_stride(type, i);
            //}
            std::cout << "\n";
        }
        descriptor.uniforms.push_back(uniform);
    }

    //for (auto& image : resources.sampled_images) {
    //    Sampler sampler{
    //        image.name,
    //        compiler.get_decoration(image.id, spv::DecorationDescriptorSet),
    //        compiler.get_decoration(image.id, spv::DecorationBinding)};

    //    descriptor.samplers.push_back(sampler);
    //}

    //for (auto& input : resources.stage_inputs) {
    //    std::cout << "Input name: " << input.name << "\n";
    //}

    //for (auto& output : resources.stage_outputs) {
    //    std::cout << "Output name: " << output.name << "\n";
    //}

    //printDescriptor(descriptor);

    return descriptor;
}

vk::PipelineLayout createPipelineLayout(
    Device& device,
    vk::DescriptorSetLayout descriptorSetLayout1,
    vk::DescriptorSetLayout descriptorSetLayout2,
    vk::DescriptorSetLayout descriptorSetLayout3,
    const nlohmann::json& json)
{
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 16;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};

    std::vector<vk::DescriptorSetLayout> layouts{};
    if (descriptorSetLayout1) {
        layouts.push_back(descriptorSetLayout1);
    }
    if (descriptorSetLayout2) {
        layouts.push_back(descriptorSetLayout2);
    }
    if (descriptorSetLayout3) {
        layouts.push_back(descriptorSetLayout3);
    }

    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    vk::PipelineLayout pipelineLayout =
        static_cast<vk::Device>(device).createPipelineLayout(pipelineLayoutInfo);
    return pipelineLayout;
}

vk::ShaderModule createShaderFromFile(vk::Device device, std::string filename)
{
    auto code = readFile(filename);
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    vk::ShaderModule shaderModule = device.createShaderModule(createInfo, nullptr);
    return shaderModule;
}

vk::ShaderModule createShaderFromFileByKey(
    vk::Device device, const nlohmann::json& json, std::string keyname)
{
    if (json.contains(keyname)) {
        return createShaderFromFile(device, json[keyname]);
    } else {
        return nullptr;
    }
}

vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
    const nlohmann::json& json)
{
    vk::PipelineRasterizationStateCreateInfo info{};

    if (hasKey(json, "polygonMode")) {
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

    if (hasKey(json, "cullMode")) {
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

vk::CompareOp compareOp(const std::string& op)
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

vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo(
    const nlohmann::json& json)
{
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};

    if (hasKey(json, "depthTestEnable")) {
        depthStencil.depthTestEnable = json["depthTestEnable"];
    }
    if (hasKey(json, "depthWriteEnable")) {
        depthStencil.depthWriteEnable = json["depthWriteEnable"];
    }
    if (hasKey(json, "depthCompareOp")) {
        depthStencil.depthCompareOp = compareOp(json["depthCompareOp"]);
    }

    return depthStencil;
}

struct ColorBlendStateCreateInfo {
    std::vector<vk::PipelineColorBlendAttachmentState> attachments;
    vk::PipelineColorBlendStateCreateInfo info;
};

ColorBlendStateCreateInfo colorBlendStateCreateInfo(const nlohmann::json& json)
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

vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo(
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

ViewportStateCreateInfo viewportStateCreateInfo(
    const nlohmann::json& json, vk::Extent2D swapChainExtent)
{
    ViewportStateCreateInfo info{};
    info.viewports.resize(1);
    info.scissors.resize(1);

    info.viewports[0].x = 0.0f;
    info.viewports[0].y = 0.0f;
    info.viewports[0].width = static_cast<float>(swapChainExtent.width);
    info.viewports[0].height = static_cast<float>(swapChainExtent.height);
    info.viewports[0].minDepth = 0.0f;
    info.viewports[0].maxDepth = 1.0f;

    info.scissors[0].offset = vk::Offset2D(0, 0);
    info.scissors[0].extent = swapChainExtent;

    info.info.viewportCount = 1;
    info.info.pViewports = &info.viewports[0];
    info.info.scissorCount = 1;
    info.info.pScissors = &info.scissors[0];

    return info;
}

vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(
    const vk::VertexInputBindingDescription& bindingDescription,
    const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions)
{
    vk::PipelineVertexInputStateCreateInfo info{};
    info.vertexBindingDescriptionCount = 1;
    info.pVertexBindingDescriptions = &bindingDescription;
    info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    info.pVertexAttributeDescriptions = attributeDescriptions.data();
    return info;
}

vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo()
{
    vk::PipelineInputAssemblyStateCreateInfo info{};
    info.topology = vk::PrimitiveTopology::eTriangleList;
    info.primitiveRestartEnable = false;
    return info;
}

std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(
    Device& device, const nlohmann::json& json)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};

    if (hasKey(json, "vertexShader")) {
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = createShaderFromFile(device, json["vertexShader"]);
        vertShaderStageInfo.pName = "main";
        shaderStages.push_back(vertShaderStageInfo);
    }

    if (hasKey(json, "fragmentShader")) {
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = createShaderFromFile(device, json["fragmentShader"]);
        fragShaderStageInfo.pName = "main";
        shaderStages.push_back(fragShaderStageInfo);
    }

    return shaderStages;
}

vk::Pipeline createPipeline(
    Device& device,
    FramebufferSet& framebufferSet,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    vk::Extent2D swapChainExtent,
    vk::PipelineLayout pipelineLayout,
    const nlohmann::json& json)
{
    vk::GraphicsPipelineCreateInfo pipelineInfo{};

    auto shaderStages = createShaderStages(device, json);
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();

    auto inputAssemblyState = inputAssemblyStateCreateInfo();
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;

    auto vertexInputState =
        vertexInputStateCreateInfo(bindingDescription, attributeDescriptions);
    pipelineInfo.pVertexInputState = &vertexInputState;

    auto viewportState = viewportStateCreateInfo(json, swapChainExtent);
    pipelineInfo.pViewportState = &viewportState.info;

    auto rasterizationState = rasterizationStateCreateInfo(json);
    pipelineInfo.pRasterizationState = &rasterizationState;

    auto multisampleState = multisampleStateCreateInfo(json);
    pipelineInfo.pMultisampleState = &multisampleState;

    auto depthStencil = depthStencilStateCreateInfo(json);
    pipelineInfo.pDepthStencilState = &depthStencil;

    auto colorBlendState = colorBlendStateCreateInfo(json);
    pipelineInfo.pColorBlendState = &colorBlendState.info;

    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = framebufferSet.renderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;
    pipelineInfo.basePipelineIndex = -1;

    vk::Pipeline pipeline = static_cast<vk::Device>(device).createGraphicsPipeline(
        nullptr, pipelineInfo, nullptr);

    for (vk::PipelineShaderStageCreateInfo info : shaderStages) {
        static_cast<vk::Device>(device).destroyShaderModule(info.module);
    }

    return pipeline;
}

static DescriptorSet createDescriptorSet(
    DescriptorManager& descriptorManager, Texture* texture)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0,
         vk::DescriptorType::eCombinedImageSampler,
         1,
         vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    if (texture) {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = texture->imageView();
        imageInfo.sampler = texture->sampler();
        descriptorSet.writeDescriptors({{0, 0, 1, &imageInfo}});
    }

    return descriptorSet;
}

static Texture* createTextureFromFile(
    TextureManager& textureManager, const nlohmann::json& json)
{
    if (hasKey(json, "texture")) {
        return &textureManager.createTextureFromFile(
            json["texture"], vk::SamplerAddressMode::eRepeat);
    } else {
        return nullptr;
    }
}

Pipeline::Pipeline(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    vk::DescriptorSetLayout descriptorSetLayout1,
    vk::DescriptorSetLayout descriptorSetLayout2,
    const nlohmann::json& json)
    : mDevice{device},
      mFramebufferSet{mDevice, swapChain, depthTexture, json},
      mTexture{createTextureFromFile(textureManager, json)},
      mDescriptorSet{createDescriptorSet(descriptorManager, mTexture)},
      mPipelineLayout{createPipelineLayout(
          mDevice,
          descriptorSetLayout1,
          mDescriptorSet.layout(),
          descriptorSetLayout2,
          json)},
      mPipeline{createPipeline(
          mDevice,
          mFramebufferSet,
          bindingDescription,
          attributeDescriptions,
          swapChain.extent(),
          mPipelineLayout,
          json)}
{
    auto info = parse(json);
    createDescriptorSetLayout(mDevice, info);
}

Pipeline::~Pipeline()
{
    static_cast<vk::Device>(mDevice).destroyPipeline(mPipeline);
    static_cast<vk::Device>(mDevice).destroyPipelineLayout(mPipelineLayout);
}
