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
        std::vector<Texture> textures,
        vk::ShaderModule vertexShader,
        vk::ShaderModule fragmentShader,
        MaterialUsage materialUsage);

    //Material(const Material& rhs) = delete;

    Material(Material&& rhs)
        : mDevice{rhs.mDevice},
          mTextures{std::move(rhs.mTextures)},
          mDescriptorSet{rhs.mDescriptorSet},
          mFramebufferSet{rhs.mFramebufferSet},
          mVertexShader{rhs.mVertexShader},
          mFragmentShader{rhs.mFragmentShader},
          mMaterialUsage{rhs.mMaterialUsage}
    {
        //rhs.mVertexShader = nullptr;
        //rhs.mFragmentShader = nullptr;
    }

    ~Material();

    FramebufferSet& framebufferSet()
    {
        return mFramebufferSet;
    }

    vk::ShaderModule vertexShader()
    {
        return std::move(mVertexShader);
    }

    vk::ShaderModule fragmentShader()
    {
        return mFragmentShader;
    }

    Texture& texture(size_t index)
    {
        return mTextures[index];
    }

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

    MaterialUsage materialUsage()
    {
        return mMaterialUsage;
    }

    size_t textureCount()
    {
        return mTextures.size();
    }

private:
    Device& mDevice;
    std::vector<Texture> mTextures;
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
