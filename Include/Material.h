#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Texture.h"
#include "../Include/Pipeline.h"
#include "../Include/SwapChain.h"

class Material {
public:
    Material(
        Device& device,
        DescriptorManager& descriptorManager,
        Buffer& uniformBuffer,
        vk::DeviceSize uniformBufferSize,
        SwapChain& swapChain,
        Texture& depthImage,
        vk::VertexInputBindingDescription bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
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

    Texture& texture()
    {
        return mTexture;
    }

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

private:
    Texture mTexture;
    DescriptorSet mDescriptorSet;
    FramebufferSet mFramebufferSet;
    Pipeline mPipeline;
};
