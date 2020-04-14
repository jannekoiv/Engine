
#pragma once

#include "../Include/Material.h"

class Device;
class DescriptorSetLayout;
class RenderPass;
class Material;

class Pipeline {
public:
    Pipeline(const Pipeline&) = delete;

    Pipeline(Pipeline&& rhs);

    Pipeline(
        Device& device,
        Material& material,
        vk::DescriptorSetLayout descriptorSetLayout,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        const vk::Extent2D& swapChainExtent,
        const MaterialUsage usage);

    ~Pipeline();

    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline& operator=(Pipeline&&) = delete;

    operator vk::Pipeline() const
    {
        return mPipeline;
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
