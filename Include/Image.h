#pragma once

#include "../Include/Base.h"

class Device;

class Image {
public:
    Image(
        Device& device,
        vk::Extent3D extent,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags memoryProperties);

    ~Image();

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

private:
    Device& mDevice;
    vk::Image mImage;
    vk::DeviceMemory mMemory;
    vk::Extent3D mExtent;
    vk::Format mFormat;
};
