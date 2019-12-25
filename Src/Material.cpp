
#define STB_IMAGE_IMPLEMENTATION

#include "../Include/Material.h"
#include "../Include/Device.h"
#include "stb_image.h"

Image createTexture(Device& device, std::string filename)
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

    Image image(
        device,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

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

Material::Material(
    Device& device,
    SwapChain& swapChain,
    Image& depthImage,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    vk::DescriptorSetLayout descriptorSetLayout,
    std::string vertexShaderFilename,
    std::string fragmentShaderFilename,
    std::string textureFilename)
    : mBindingDescription{bindingDescription},
      mAttributeDescriptions{attributeDescriptions},
      mDescriptorSetLayout{descriptorSetLayout},
      mFramebufferSet{device, swapChain, depthImage, vk::AttachmentLoadOp::eLoad},
      mPipeline{
          device,
          mBindingDescription,
          mAttributeDescriptions,
          mDescriptorSetLayout,
          vertexShaderFilename,
          fragmentShaderFilename,
          swapChain.extent(),
          mFramebufferSet.renderPass()},
      mTexture{createTexture(device, textureFilename)}
{
}
