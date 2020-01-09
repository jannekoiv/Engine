
#define STB_IMAGE_IMPLEMENTATION

#include "../Include/Material.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "stb_image.h"
#include <fstream>

Material::Material(Material&& rhs)
    : mDevice{rhs.mDevice},
      mTexture{rhs.mTexture},
      mDescriptorSet{rhs.mDescriptorSet},
      mFramebufferSet{rhs.mFramebufferSet},
      mVertexShader{rhs.mVertexShader},
      mFragmentShader{rhs.mFragmentShader}
{
    rhs.mVertexShader = nullptr;
    rhs.mFragmentShader = nullptr;
}

Texture createTexture(Device& device, std::string filename)
{
    const int bytesPerPixel = 4;
    int width = 0;
    int height = 0;
    int channelCount = 0;

    stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channelCount, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    vk::DeviceSize imageSize = width * height * bytesPerPixel;

    Buffer stagingBuffer(
        device,
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = nullptr;
    static_cast<vk::Device>(device).mapMemory(stagingBuffer.memory(), 0, imageSize, {}, &data);
    memcpy(data, pixels, imageSize);
    static_cast<vk::Device>(device).unmapMemory(stagingBuffer.memory());
    stbi_image_free(pixels);

    Texture texture{
        device,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eRepeat};

    texture.transitionLayout(vk::ImageLayout::eTransferDstOptimal);
    stagingBuffer.copyToImage(texture);
    texture.transitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    return texture;
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

vk::ShaderModule createShader(vk::Device device, std::string filename)
{
    auto code = readFile(filename);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    vk::ShaderModule shaderModule = device.createShaderModule(createInfo, nullptr);
    return shaderModule;
}

Material::Material(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthImage,
    Texture&& texture,
    vk::ShaderModule vertexShader,
    vk::ShaderModule fragmentShader)
    : mDevice{device},
      mTexture{texture},
      mDescriptorSet{createDescriptorSet(descriptorManager, *this)},
      mFramebufferSet{device, swapChain, depthImage, vk::AttachmentLoadOp::eLoad},
      mVertexShader{vertexShader},
      mFragmentShader{fragmentShader}
{
    //std::cout << "Material constructor\n";
}

Material::~Material()
{
    //std::cout << "Material destructor\n";
    static_cast<vk::Device>(mDevice).destroyShaderModule(mVertexShader);
}

Material createMaterialFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthImage,
    std::string filename)
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

    auto textureFilename = readString(file);
    auto texture = createTexture(device, textureFilename);

    auto vertexShaderFilename = readString(file);
    auto vertexShader = createShader(device, vertexShaderFilename);

    auto fragmentShaderFilename = readString(file);
    auto fragmentShader = createShader(device, fragmentShaderFilename);

    return Material(
        device, descriptorManager, swapChain, depthImage, createTexture(device, textureFilename), vertexShader, fragmentShader);
}
