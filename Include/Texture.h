#pragma once

#include "../Include/Base.h"

class Device;

class Texture {
public:
    Texture(const Texture&) = delete;

    Texture(Texture&& rhs);

    Texture(
        Device& device,
        vk::ImageViewType type,
        uint32_t layerCount,
        vk::Extent3D extent,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags memoryProperties,
        vk::SamplerAddressMode addressMode);

    ~Texture();

    Texture& operator=(const Texture&) = delete;

    Texture& operator=(Texture&&) = delete;

    Device& device() const
    {
        return mDevice;
    }

    vk::ImageViewType type() const
    {
        return mType;
    }

    uint32_t layerCount() const
    {
        return mLayerCount;
    }

    vk::Extent3D extent() const
    {
        return mExtent;
    }

    vk::Format format() const
    {
        return mFormat;
    }

    vk::DeviceMemory memory() const
    {
        return mMemory;
    }

    vk::Image image() const
    {
        return mImage;
    }

    vk::ImageView imageView() const
    {
        return mImageView;
    }

    vk::Sampler sampler() const
    {
        return mSampler;
    }

    void transitionLayout(
        vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer externalCommandBuffer = nullptr);

private:
    Device& mDevice;
    vk::ImageViewType mType;
    uint32_t mLayerCount;
    vk::Extent3D mExtent;
    vk::Format mFormat;
    vk::Image mImage;
    vk::DeviceMemory mMemory;
    vk::ImageView mImageView;
    vk::Sampler mSampler;
};

//Texture createTextureFromFile(Device& device, std::string filename, vk::SamplerAddressMode addressMode);
//Texture createCubeTextureFromFile(Device& device, std::string filename);
