

#include "../Include/Material.h"
#include "../Include/Base.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Device.h"
#include "../Include/FramebufferSet.h"
#include "../Include/TextureManager.h"
#include <fstream>
#include <json.hpp>

Material::Material(Material&& rhs)
    : mDevice{rhs.mDevice},
      //      mTextures{std::move(rhs.mTextures)},
      mTexture{rhs.mTexture},
      mDescriptorSet{std::move(rhs.mDescriptorSet)},
      mFramebufferSet{std::move(rhs.mFramebufferSet)},
      mVertexShader{rhs.mVertexShader},
      mFragmentShader{rhs.mFragmentShader},
      mMaterialUsage{rhs.mMaterialUsage}
{
    rhs.mVertexShader = nullptr;
    rhs.mFragmentShader = nullptr;
    //rhs.mTextures.clear();
}

DescriptorSet createDescriptorSet(DescriptorManager& descriptorManager, Material& material)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    //if (material.textureCount() > 0) {
    if (material.texture()) {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        //imageInfo.imageview = material.texture(0).imageView();
        //imageInfo.sampler = material.texture(0).sampler();
        imageInfo.imageView = material.texture()->imageView();
        imageInfo.sampler = material.texture()->sampler();
        descriptorSet.writeDescriptors({{0, 0, 1, &imageInfo}});
    }
    return descriptorSet;
}

vk::ShaderModule createShaderFromFile(vk::Device device, std::string filename)
{
    auto code = readFile(filename);
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    vk::ShaderModule shaderModule = device.createShaderModule(createInfo, nullptr);
    return shaderModule;
}

Material::Material(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    //std::vector<Texture> textures,
    Texture* texture,
    vk::ShaderModule vertexShader,
    vk::ShaderModule fragmentShader,
    MaterialUsage materialUsage)
    : mDevice{device},
      //mTextures{std::move(textures)},
      mTexture{texture},
      mFramebufferSet{mDevice, swapChain, depthTexture, materialUsage},
      mVertexShader{vertexShader},
      mFragmentShader{fragmentShader},
      mMaterialUsage{materialUsage},
      mDescriptorSet{createDescriptorSet(descriptorManager, *this)}
{
    std::cout << "Material constructed.\n";
}

Material::~Material()
{
    static_cast<vk::Device>(mDevice).destroyShaderModule(mVertexShader);
    static_cast<vk::Device>(mDevice).destroyShaderModule(mFragmentShader);
}

Material createMaterialFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    std::string filename,
    MaterialUsage materialUsage)
{
    using Json = nlohmann::json;
    std::ifstream file{filename};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open material file!");
    }
    Json json;
    file >> json;

    auto& texture = textureManager.createTextureFromFile(json["texture"], vk::SamplerAddressMode::eRepeat);

    auto vertexShader = createShaderFromFile(device, json["vertexShader"]);
    auto fragmentShader = createShaderFromFile(device, json["fragmentShader"]);

    return Material{
        device,
        descriptorManager,
        swapChain,
        depthTexture,
        //std::move(textures),
        &texture,
        vertexShader,
        fragmentShader,
        materialUsage};
}
