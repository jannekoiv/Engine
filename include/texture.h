#pragma once

#include "base.h"

class Device;

class Texture {
public:
    Texture(const Texture&) = delete;

    Texture(Texture&& rhs);

    Texture(
        Device& device,
        vk::ImageViewType type,
        uint32_t layer_count,
        vk::Extent3D extent,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags memory_properties,
        vk::SamplerAddressMode address_mode);

    ~Texture();

    Texture& operator=(const Texture&) = delete;

    Texture& operator=(Texture&&) = delete;

    Device& device() const
    {
        return _device;
    }

    vk::ImageViewType type() const
    {
        return _type;
    }

    uint32_t layer_count() const
    {
        return _layer_count;
    }

    vk::Extent3D extent() const
    {
        return _extent;
    }

    vk::Format format() const
    {
        return _format;
    }

    vk::DeviceMemory memory() const
    {
        return _memory;
    }

    vk::Image image() const
    {
        return _image;
    }

    vk::ImageView image_view() const
    {
        return _image_view;
    }

    vk::Sampler sampler() const
    {
        return _sampler;
    }

    void transition_layout(
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::CommandBuffer external_command_buffer = nullptr);

private:
    Device& _device;
    vk::ImageViewType _type;
    uint32_t _layer_count;
    vk::Extent3D _extent;
    vk::Format _format;
    vk::Image _image;
    vk::DeviceMemory _memory;
    vk::ImageView _image_view;
    vk::Sampler _sampler;
};

//Texture createTextureFromFile(Device& device, std::string filename, vk::SamplerAddressMode addressMode);
//Texture createCubeTextureFromFile(Device& device, std::string filename);
