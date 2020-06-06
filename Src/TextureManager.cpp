

#define STB_IMAGE_IMPLEMENTATION

#include "../Include/TextureManager.h"
#include "../Include/Buffer.h"
#include <stb_image.h>

TextureContainer::TextureContainer(std::string filename, Texture texture)
    : filename{filename}, texture{std::move(texture)}
{
}

TextureManager::TextureManager(Device& device) : mDevice{device}
{
}

Texture& TextureManager::createTextureFromFile(
    std::string filename, vk::SamplerAddressMode addressMode)
{
    if (filename.empty()) {
        throw std::runtime_error("Failed to load texture image!");
    }

    for (auto& texture : mTextures) {
        if (texture.filename == filename) {
            return texture.texture;
        }
    }

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
        mDevice,
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = stagingBuffer.mapMemory();
    memcpy(data, pixels, imageSize);
    stagingBuffer.unmapMemory();
    stbi_image_free(pixels);

    Texture texture{
        mDevice,
        vk::ImageViewType::e2D,
        1,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        addressMode};

    texture.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    stagingBuffer.copyToTexture(texture, 0);
    texture.transitionLayout(
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    return mTextures.emplace_back(filename, std::move(texture)).texture;
}

Texture& TextureManager::createCubeTextureFromFile(std::array<std::string, 6> filenames)
{
    std::vector<Buffer> stagingBuffers{};
    std::string combinedFilenames;
    int width = 0;
    int height = 0;

    for (int face = 0; face < 6; face++) {
        const int bytesPerPixel = 4;
        int channelCount = 0;

        stbi_uc* pixels =
            stbi_load(filenames[face].data(), &width, &height, &channelCount, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        vk::DeviceSize imageSize = width * height * bytesPerPixel;

        stagingBuffers.emplace_back(
            mDevice,
            imageSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        Buffer& stagingBuffer = stagingBuffers.back();

        void* data = stagingBuffer.mapMemory();
        memcpy(data, pixels, imageSize);
        stagingBuffer.unmapMemory();
        stbi_image_free(pixels);

        combinedFilenames += filenames[face];
    }

    Texture texture{
        mDevice,
        vk::ImageViewType::eCube,
        6,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eClampToEdge};

    texture.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    for (int face = 0; face < 6; face++) {
        stagingBuffers[face].copyToTexture(texture, face);
    }
    texture.transitionLayout(
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    return mTextures.emplace_back(combinedFilenames, std::move(texture)).texture;
}
