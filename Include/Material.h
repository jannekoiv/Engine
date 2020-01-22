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
        SwapChain& swapChain,
        Texture* depthTexture,
        Texture& texture,
        vk::ShaderModule vertexShader,
        vk::ShaderModule fragmentShader,
        MaterialUsage materialUsage);

    Material(const Material& rhs) = delete;

    Material(Material&& rhs);

    ~Material();

    FramebufferSet& framebufferSet()
    {
        return mFramebufferSet;
    }

    vk::ShaderModule vertexShader()
    {
        return mVertexShader;
    }

    vk::ShaderModule fragmentShader()
    {
        return mFragmentShader;
    }

    Texture& texture()
    {
        return mTexture;
    }

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

    MaterialUsage materialUsage()
    {
        return mMaterialUsage;
    }

private:
    Device& mDevice;
    Texture mTexture;
    FramebufferSet mFramebufferSet;
    vk::ShaderModule mVertexShader;
    vk::ShaderModule mFragmentShader;
    MaterialUsage mMaterialUsage;
    DescriptorSet mDescriptorSet;
};

vk::ShaderModule createShaderFromFile(vk::Device device, std::string filename);

Material createMaterialFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    std::string filename,
    MaterialUsage materialUsage);
