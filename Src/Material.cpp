

#include "../Include/Material.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include <fstream>

Material::Material(Material&& rhs)
    : mDevice{rhs.mDevice},
      mTexture{rhs.mTexture},
      mDescriptorSet{rhs.mDescriptorSet},
      mFramebufferSet{rhs.mFramebufferSet},
      mVertexShader{rhs.mVertexShader},
      mFragmentShader{rhs.mFragmentShader},
      mMaterialUsage{rhs.mMaterialUsage}
{
    rhs.mVertexShader = nullptr;
    rhs.mFragmentShader = nullptr;
}

DescriptorSet createDescriptorSet(DescriptorManager& descriptorManager, Material& material)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = material.texture().view();
    imageInfo.sampler = material.texture().sampler();

    descriptorSet.writeDescriptors({{0, 0, 1, &imageInfo}});
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
    Texture& texture,
    vk::ShaderModule vertexShader,
    vk::ShaderModule fragmentShader,
    MaterialUsage materialUsage)
    : mDevice{device},
      mTexture{texture},
      mFramebufferSet{device, swapChain, depthTexture, &texture, materialUsage},
      mVertexShader{vertexShader},
      mFragmentShader{fragmentShader},
      mMaterialUsage{materialUsage},
      mDescriptorSet{createDescriptorSet(descriptorManager, *this)}
{
}

Material::~Material()
{
    static_cast<vk::Device>(mDevice).destroyShaderModule(mVertexShader);
}

Material createMaterialFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture* depthTexture,
    std::string filename,
    MaterialUsage materialUsage)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open material file!");
    }

    auto header = readString(file);
    if (header != "paskaformaatti 1.0") {
        std::cout << "HEADER\n";
        throw std::runtime_error("Header file not matching!");
    }

    auto texture = createTextureFromFile(device, readString(file));
    auto vertexShader = createShaderFromFile(device, readString(file));
    auto fragmentShader = createShaderFromFile(device, readString(file));

    return Material{
        device,
        descriptorManager,
        swapChain,
        depthTexture,
        texture,
        vertexShader,
        fragmentShader,
        materialUsage};
}
