
#pragma once

#include "../Include/Base.h"

class Device;
class DescriptorSetLayout;
class RenderPass;

class Pipeline {
public:
    Pipeline(
        Device& device,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        vk::DescriptorSetLayout descriptorSetLayout,
        std::string vertexShaderFilename,
        std::string fragmentShaderFilename,
        const vk::Extent2D& swapChainExtent,
        const vk::RenderPass& renderPass);

    operator vk::Pipeline() const
    {
        return mPipeline;
    }

    Device& device() const
    {
        return mDevice;
    }

    vk::PipelineLayout layout() const
    {
        return mPipelineLayout;
    }

private:
    Device& mDevice;
    vk::PipelineLayout mPipelineLayout;
    vk::Pipeline mPipeline;
};
