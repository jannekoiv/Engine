
#pragma once

#include "../Include/Base.h"

class Device;
class DescriptorSetLayout;
class RenderPass;
class Material;

class Pipeline {
public:
    //Pipeline(
    //    Device& device,
    //    vk::VertexInputBindingDescription bindingDescription,
    //    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    //    vk::DescriptorSetLayout descriptorSetLayout,
    //    std::string vertexShaderFilename,
    //    std::string fragmentShaderFilename,
    //    const vk::Extent2D& swapChainExtent,
    //    const vk::RenderPass& renderPass);

    Pipeline(
        Device& device,
        Material& material,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        const vk::Extent2D& swapChainExtent);

    operator vk::Pipeline() const
    {
        return mPipeline;
    }

    vk::PipelineLayout layout() const
    {
        return mPipelineLayout;
    }

private:
    vk::PipelineLayout mPipelineLayout;
    vk::Pipeline mPipeline;
};
