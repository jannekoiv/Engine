#pragma once

#include "ext_includes.h"

class Device;
class Texture;

class Buffer {
public:
    Buffer(const Buffer&) = delete;

    Buffer(Buffer&& buffer);

    Buffer(
        Device& device,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags memory_properties);

    ~Buffer();

    Buffer& operator=(const Buffer&) = delete;

    Buffer& operator=(Buffer&& buffer) = delete;

    operator vk::Buffer() const
    {
        return _buffer;
    }

    vk::DeviceSize size()
    {
        return _size;
    }

    vk::DeviceMemory& memory()
    {
        return _memory;
    }

    // void copy(Buffer& dst_buffer, vk::DeviceSize size);
    // void copy(Buffer& dst_buffer);
    void copy(vk::CommandPool command_pool, Buffer& dst_buffer, vk::DeviceSize size);
    void copy(vk::CommandPool command_pool, Buffer& dst_buffer);

    void copy_to_texture(
        vk::CommandPool command_pool,
        Texture& dst_texture,
        int layer,
        vk::Offset3D offset,
        vk::Extent3D extent);
    void copy_to_texture(vk::CommandPool command_pool, Texture& dst_texture, int layer);

    void* map_memory(vk::DeviceSize offset, vk::DeviceSize size);
    void* map_memory();
    void unmap_memory();

private:
    Device& _device;
    vk::DeviceSize _size;
    vk::Buffer _buffer;
    vk::DeviceMemory _memory;
};
