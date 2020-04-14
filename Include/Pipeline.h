
#pragma once

#include "../Include/FramebufferSet.h"
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
        SwapChain& swapChain,
        Texture* depthTexture,
        vk::DescriptorSetLayout descriptorSetLayout,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        const MaterialUsage usage,
        const nlohmann::json& json);

    ~Pipeline();

    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline& operator=(Pipeline&&) = delete;

    operator vk::Pipeline() const
    {
        return mPipeline;
    }

    FramebufferSet& framebufferSet()
    {
        return mFramebufferSet;
    }

    vk::PipelineLayout layout() const
    {
        return mPipelineLayout;
    }

private:
    Device& mDevice;
    FramebufferSet mFramebufferSet;
    vk::PipelineLayout mPipelineLayout;
    vk::Pipeline mPipeline;
};
