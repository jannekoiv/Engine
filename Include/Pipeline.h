
#pragma once

#include "../Include/Material.h"

class Device;
class DescriptorSetLayout;
class RenderPass;
class Material;

class Pipeline {
public:
    Pipeline(
        Device& device,
        Material& material,
        vk::DescriptorSetLayout descriptorSetLayout,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        const vk::Extent2D& swapChainExtent,
        const MaterialUsage usage);

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
