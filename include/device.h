#pragma once

#include "base.h"

struct QueueFamilyIndices {
    int graphics = -1;
    int present = -1;
    bool is_complete()
    {
        return graphics >= 0 && present >= 0;
    }
};

struct SwapChainSupportDetails {
    SwapChainSupportDetails(vk::SurfaceKHR surface, vk::PhysicalDevice device);

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

class Device {
public:
    Device(const Device&) = delete;

    Device(Device&&) = delete;

    Device(GLFWwindow* window, bool enable_validation_layer);

    ~Device();

    Device& operator=(const Device&) = delete;

    Device& operator=(Device&&) = delete;

    operator vk::Device() const
    {
        return _device;
    }

    vk::SurfaceKHR surface() const
    {
        return _surface;
    }

    vk::PhysicalDevice physical_device() const
    {
        return _physical_device;
    }

    QueueFamilyIndices queue_family_indices() const
    {
        return _queue_family_indices;
    }

    vk::Queue graphics_queue() const
    {
        return _graphics_queue;
    }

    vk::Queue present_queue() const
    {
        return _present_queue;
    }

    vk::CommandPool command_pool() const
    {
        return _command_pool;
    }

    uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties) const;

    vk::CommandBuffer create_and_begin_command_buffer();

    void flush_and_free_command_buffer(vk::CommandBuffer command_buffer);

    vk::Format find_supported_format(
        const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    vk::Instance instance() {
	return _instance;
    }

    vk::Device device() {
	return _device;
    }

private:
    vk::Instance _instance;
    VkDebugReportCallbackEXT _debug_callback;
    vk::SurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    QueueFamilyIndices _queue_family_indices;
    vk::Device _device;
    vk::Queue _graphics_queue;
    vk::Queue _present_queue;
    vk::CommandPool _command_pool;
};

vk::Format find_depth_attachment_optimal(Device& device);
