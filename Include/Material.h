#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"

class TextureManager;

class Material {
public:
    Material(const Material&) = delete;

    Material(Material&& rhs);

    Material(
        Device& device,
        DescriptorManager& descriptorManager,
        SwapChain& swapChain,
        Texture* depthTexture,
        //std::vector<Texture> textures,
        Texture* texture,
        vk::ShaderModule vertexShader,
        vk::ShaderModule fragmentShader);

    ~Material();

    Material& operator=(const Material&) = delete;

    Material& operator=(Material&&) = delete;

    vk::ShaderModule vertexShader()
    {
        return std::move(mVertexShader);
    }

    vk::ShaderModule fragmentShader()
    {
        return mFragmentShader;
    }

    //Texture& texture(size_t index)
    //{
    //    return mTextures[index];
    //}

    Texture* texture()
    {
        return mTexture;
    }

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

    //size_t textureCount()
    //{
    //    return mTextures.size();
    //}

private:
    Device& mDevice;
    //std::vector<Texture> mTextures;
    Texture* mTexture;

    DescriptorSet mDescriptorSet;
    vk::ShaderModule mVertexShader;
    vk::ShaderModule mFragmentShader;
};

vk::ShaderModule createShaderFromFile(vk::Device device, std::string filename);

Material createMaterialFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    std::string filename);
