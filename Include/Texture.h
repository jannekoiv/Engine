#pragma once

#include "../Include/Base.h"

class Device;

class Texture {
public:
    Texture(
        Device& device,
        vk::Extent3D extent,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags memoryProperties,
        vk::SamplerAddressMode addressMode);

    ~Texture();

    operator vk::Image() const
    {
        return mImage;
    }

    Device& device() const
    {
        return mDevice;
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

    vk::ImageView view() const
    {
        return mView;
    }

    vk::Sampler sampler() const
    {
        return mSampler;
    }

    void transitionLayout(
        vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

private:
    Device& mDevice;
    vk::Extent3D mExtent;
    vk::Format mFormat;
    vk::Image mImage;
    vk::DeviceMemory mMemory;
    vk::ImageView mView;
    vk::Sampler mSampler;
};
