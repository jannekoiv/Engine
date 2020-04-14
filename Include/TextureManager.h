#pragma once

#include "../Include/Base.h"
#include "../Include/Texture.h"

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

    Texture& createTextureFromFile(std::string filename, vk::SamplerAddressMode addressMode);

    Texture& createCubeTextureFromFile(std::array<std::string, 6> filenames);

private:
    Device& mDevice;
    std::vector<TextureContainer> mTextures;
};
