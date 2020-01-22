#pragma once

#include "../Include/Base.h"

class Device;
class Texture;

class Buffer {
public:
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&& buffer);

    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&& buffer) = delete;

    Buffer(
        Device& device,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags memoryProperties);

    ~Buffer();

    operator vk::Buffer() const
    {
        return mBuffer;
    }

    vk::DeviceSize size()
    {
        return mSize;
    }

    const vk::DeviceMemory& memory() const
    {
        return mMemory;
    }

    void copy(Buffer& dstBuffer, vk::DeviceSize size);
    void copy(Buffer& dstBuffer);
    void copyToTexture(Texture& dstTexture, int layer, vk::Offset3D offset, vk::Extent3D extent);
    void copyToTexture(Texture& dstTexture, int layer);

    void* mapMemory(vk::DeviceSize offset, vk::DeviceSize size);
    void* mapMemory();
    void unmapMemory();

private:
    Device& mDevice;
    vk::DeviceSize mSize;
    vk::Buffer mBuffer;
    vk::DeviceMemory mMemory;
};
