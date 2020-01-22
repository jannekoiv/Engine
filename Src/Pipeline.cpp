
#include "../Include/Pipeline.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "../Include/Material.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vulkan/vulkan.hpp>

vk::PipelineLayout createPipelineLayout(
    Device& device, Material& material, vk::DescriptorSetLayout descriptorSetLayout)
{
    std::array<vk::DescriptorSetLayout, 2> layouts = {
        descriptorSetLayout, material.descriptorSet().layout()};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    vk::PipelineLayout pipelineLayout =
        static_cast<vk::Device>(device).createPipelineLayout(pipelineLayoutInfo, nullptr);
    return pipelineLayout;
}

vk::Pipeline createPipeline(
    Device& device,
    Material& material,
    vk::DescriptorSetLayout descriptorSetLayout,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    vk::Extent2D swapChainExtent,
    vk::PipelineLayout pipelineLayout,
    const MaterialUsage usage)
{
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = material.vertexShader();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = material.fragmentShader();
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo,
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = false;

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapChainExtent;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.depthClampEnable = false;
    rasterizer.rasterizerDiscardEnable = false;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = false;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;

    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.sampleShadingEnable = false;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = false;
    multisampling.alphaToOneEnable = false;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eB;
    colorBlendAttachment.blendEnable = false;

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments = {
        colorBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOpEnable = false;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();

    vk::PipelineDepthStencilStateCreateInfo depthStencil;

    depthStencil.depthTestEnable = true;
    depthStencil.depthWriteEnable = true;

    if (usage == MaterialUsage::Skybox) {
        depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
    } else if (usage == MaterialUsage::Model) {
        depthStencil.depthCompareOp = vk::CompareOp::eLess;
    } else if (usage == MaterialUsage::Quad) {
        depthStencil.depthTestEnable = false;
        depthStencil.depthWriteEnable = false;
    }

    depthStencil.depthBoundsTestEnable = false;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = false;
    depthStencil.front = vk::StencilOpState{};
    depthStencil.back = vk::StencilOpState{};

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = material.framebufferSet().renderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;
    pipelineInfo.basePipelineIndex = -1;

    vk::Pipeline pipeline =
        static_cast<vk::Device>(device).createGraphicsPipeline(nullptr, pipelineInfo, nullptr);
    return pipeline;
}

Pipeline::Pipeline(
    Device& device,
    Material& material,
    vk::DescriptorSetLayout descriptorSetLayout,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    const vk::Extent2D& swapChainExtent,
    const MaterialUsage usage)
    : mPipelineLayout{createPipelineLayout(device, material, descriptorSetLayout)},
      mPipeline{createPipeline(
          device,
          material,
          descriptorSetLayout,
          bindingDescription,
          attributeDescriptions,
          swapChainExtent,
          mPipelineLayout,
          usage)}
{
}
