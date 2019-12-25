#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Framebuffer.h"
#include "../Include/Image.h"
#include "../Include/Pipeline.h"
#include "../Include/Sampler.h"
#include "../Include/SwapChain.h"

class Material {
public:
    Material(
        Device& device,
        SwapChain& swapChain,
        Image& depthImage,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
        vk::DescriptorSetLayout descriptorSetLayout,
        std::string vertexShaderFilename,
        std::string fragmentShaderFilename,
        std::string textureFilename);

    FramebufferSet& framebufferSet()
    {
        return mFramebufferSet;
    }
    
    Pipeline& pipeline()
    {
        return mPipeline;
    }

private:
    vk::VertexInputBindingDescription mBindingDescription;
    std::vector<vk::VertexInputAttributeDescription> mAttributeDescriptions;
    vk::DescriptorSetLayout mDescriptorSetLayout;
    FramebufferSet mFramebufferSet;
    Pipeline mPipeline;
    Image mTexture;
};
