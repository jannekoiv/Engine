#pragma once

#include "ext_includes.h"
#include "texture.h"

class Device;

struct TextureContainer {
    TextureContainer(std::string filename, Texture texture);

    std::string filename;
    Texture texture;
};

class TextureManager {
public:
    TextureManager(const TextureManager&) = delete;

    TextureManager(TextureManager&&) = delete;

    TextureManager(Device& device);

    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager& operator=(TextureManager&&) = delete;

    Texture& create_texture_from_file(std::string filename, vk::SamplerAddressMode address_mode);

    Texture& create_cube_texture_from_file(std::array<std::string, 6> filenames);

private:
    Device& _device;
    std::vector<TextureContainer> _textures;
};
