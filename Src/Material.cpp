/*
#include "../Include/Material.h"
#include "../Include/Base.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Device.h"
#include "../Include/TextureManager.h"
#include <fstream>
#include <json.hpp>

Material::Material(Material&& rhs)
    : mDevice{rhs.mDevice}, mTexture{rhs.mTexture}, mDescriptorSet{std::move(rhs.mDescriptorSet)}
{
    rhs.mTexture = nullptr;
}

DescriptorSet createDescriptorSet(DescriptorManager& descriptorManager, Material& material)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    if (material.texture()) {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = material.texture()->imageView();
        imageInfo.sampler = material.texture()->sampler();
        descriptorSet.writeDescriptors({{0, 0, 1, &imageInfo}});
    }

    return descriptorSet;
}

Material::Material(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    Texture* texture)
    : mDevice{device},
      mTexture{texture},
      mDescriptorSet{createDescriptorSet(descriptorManager, *this)}
{
}

Material::Material(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    const nlohmann::json& json)
    : mDevice{device},
      mTexture{
          &textureManager.createTextureFromFile(json["texture"], vk::SamplerAddressMode::eRepeat)},
      mDescriptorSet{createDescriptorSet(descriptorManager, *this)}
{
}

Material::~Material()
{
}
*/
