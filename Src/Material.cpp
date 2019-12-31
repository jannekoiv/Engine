
#define STB_IMAGE_IMPLEMENTATION

#include "../Include/Material.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "stb_image.h"

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

    Texture image{
        device,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eClampToEdge};

    image.transitionLayout(
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);

    stagingBuffer.copyToImage(image);

    image.transitionLayout(
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    return image;
}

DescriptorSet createDescriptorSet(
    DescriptorManager& descriptorManager,
    Material& material,
    Buffer& uniformBuffer,
    vk::DeviceSize uniformBufferSize)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
        {1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = uniformBufferSize;

    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = material.texture().view();
    imageInfo.sampler = material.texture().sampler();

    descriptorSet.writeDescriptors({{0, 0, 1, &bufferInfo}, {1, 0, 1, &imageInfo}});

    return descriptorSet;
}

vk::ShaderModule createShaderModule(vk::Device device, std::string filename)
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
    Buffer& uniformBuffer,
    vk::DeviceSize uniformBufferSize,
    SwapChain& swapChain,
    Texture& depthImage,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    std::string vertexShaderFilename,
    std::string fragmentShaderFilename,
    std::string textureFilename)
    : mDevice{device},
      mTexture{createTexture(device, textureFilename)},
      mDescriptorSet{
          createDescriptorSet(descriptorManager, *this, uniformBuffer, uniformBufferSize)},
      mFramebufferSet{device, swapChain, depthImage, vk::AttachmentLoadOp::eLoad},
      mVertexShaderModule{createShaderModule(device, vertexShaderFilename)},
      mFragmentShaderModule{createShaderModule(device, fragmentShaderFilename)}
{
    //std::cout << "Material constructor\n";
}

Material::~Material()
{
    //std::cout << "Material destructor\n";
    static_cast<vk::Device>(mDevice).destroyShaderModule(mVertexShaderModule);
}
