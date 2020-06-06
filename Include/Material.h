/*
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
        Texture* texture);

    Material(
        Device& device,
        DescriptorManager& descriptorManager,
        TextureManager& textureManager,
        SwapChain& swapChain,
        Texture* depthTexture,
        const nlohmann::json& json);

    ~Material();

    Material& operator=(const Material&) = delete;

    Material& operator=(Material&&) = delete;

    Texture* texture()
    {
        return mTexture;
    }

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

private:
    Device& mDevice;
    Texture* mTexture;
    DescriptorSet mDescriptorSet;
};

*/
