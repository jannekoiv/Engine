
#define STB_IMAGE_IMPLEMENTATION
#include "includes.h"
#include <stb/stb_image.h>

TextureContainer::TextureContainer(std::string filename, Texture texture)
    : filename{filename}, texture{std::move(texture)}
{
}

TextureManager::TextureManager(Device& device) : _device{device}
{
}

Texture& TextureManager::create_texture_from_file(
    std::string filename, vk::SamplerAddressMode address_mode)
{
    if (filename.empty()) {
        throw std::runtime_error("Failed to load texture image!");
    }

    for (auto& texture : _textures) {
        if (texture.filename == filename) {
            return texture.texture;
        }
    }

    const int bytes_per_pixel = 4;
    int width = 0;
    int height = 0;
    int channel_count = 0;

    stbi_uc* pixels =
        stbi_load(filename.data(), &width, &height, &channel_count, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    vk::DeviceSize image_size = width * height * bytes_per_pixel;

    Buffer staging_buffer(
        _device,
        image_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, pixels, image_size);
    staging_buffer.unmap_memory();
    stbi_image_free(pixels);

    Texture texture{
        _device,
        vk::ImageViewType::e2D,
        1,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        address_mode};

    // texture.transition_layout(
    //     vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    // staging_buffer.copy_to_texture(texture, 0);
    // texture.transition_layout(
    //     vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    return _textures.emplace_back(filename, std::move(texture)).texture;
}

Texture& TextureManager::create_cube_texture_from_file(
    std::array<std::string, 6> filenames)
{
    std::vector<Buffer> staging_buffers{};
    std::string combined_filenames;
    int width = 0;
    int height = 0;

    for (int face = 0; face < 6; face++) {
        const int bytes_per_pixel = 4;
        int channel_count = 0;

        stbi_uc* pixels = stbi_load(
            filenames[face].data(), &width, &height, &channel_count, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        vk::DeviceSize image_size = width * height * bytes_per_pixel;

        staging_buffers.emplace_back(
            _device,
            image_size,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent);

        Buffer& staging_buffer = staging_buffers.back();

        void* data = staging_buffer.map_memory();
        memcpy(data, pixels, image_size);
        staging_buffer.unmap_memory();
        stbi_image_free(pixels);

        combined_filenames += filenames[face];
    }

    Texture texture{
        _device,
        vk::ImageViewType::eCube,
        6,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eClampToEdge};

    // texture.transition_layout(
    //     vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    for (int face = 0; face < 6; face++) {
        // staging_buffers[face].copy_to_texture(texture, face);
    }
    // texture.transition_layout(
    //     vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    return _textures.emplace_back(combined_filenames, std::move(texture)).texture;
}
