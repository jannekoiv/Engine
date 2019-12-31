#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/FramebufferSet.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"

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

    Material(const Material& rhs) = delete;

    Material(Material&& rhs)
        : mDevice{rhs.mDevice},
          mTexture{rhs.mTexture},
          mDescriptorSet{rhs.mDescriptorSet},
          mFramebufferSet{rhs.mFramebufferSet},
          mVertexShaderModule{rhs.mVertexShaderModule},
          mFragmentShaderModule{rhs.mFragmentShaderModule}
    {
        rhs.mVertexShaderModule = nullptr;
        rhs.mFragmentShaderModule = nullptr;
    }

    ~Material();

    FramebufferSet& framebufferSet()
    {
        return mFramebufferSet;
    }

    vk::ShaderModule vertexShaderModule()
    {
        return mVertexShaderModule;
    }

    vk::ShaderModule fragmentShaderModule()
    {
        return mFragmentShaderModule;
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
    Device& mDevice;
    Texture mTexture;
    DescriptorSet mDescriptorSet;
    FramebufferSet mFramebufferSet;
    vk::ShaderModule mVertexShaderModule;
    vk::ShaderModule mFragmentShaderModule;
};
