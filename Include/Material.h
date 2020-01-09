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
        Texture& depthImage,
        Texture&& texture,
        vk::ShaderModule vertexShader,
        vk::ShaderModule fragmentShader);

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

private:
    Device& mDevice;
    Texture mTexture;
    DescriptorSet mDescriptorSet;
    FramebufferSet mFramebufferSet;
    vk::ShaderModule mVertexShader;
    vk::ShaderModule mFragmentShader;
};

Material createMaterialFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthImage,
    std::string filename);
