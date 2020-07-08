
#pragma once

#include "../Include/DescriptorSet.h"
#include "../Include/FramebufferSet.h"

class DescriptorManager;
class TextureManager;
class Texture;

class Pipeline {
public:
    Pipeline(const Pipeline&) = delete;

    Pipeline(Pipeline&& rhs);

    Pipeline(
        Device& device,
        DescriptorManager& descriptorManager,
        TextureManager& textureManager,
        SwapChain& swapChain,
        Texture* depthTexture,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        vk::DescriptorSetLayout descriptorSetLayout1,
        vk::DescriptorSetLayout descriptorSetLayout2,
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

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

private:
    Device& mDevice;
    FramebufferSet mFramebufferSet;
    Texture* mTexture;
    DescriptorSet mDescriptorSet;
    vk::PipelineLayout mPipelineLayout;
    vk::Pipeline mPipeline;
};
