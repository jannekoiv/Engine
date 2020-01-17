#pragma once

#include "../Include/Base.h"

struct QueueFamilyIndices {
    int graphics = -1;
    int present = -1;
    bool isComplete()
    {
        return graphics >= 0 && present >= 0;
    }
};

struct SwapChainSupportDetails {
    SwapChainSupportDetails(vk::SurfaceKHR surface, vk::PhysicalDevice device);

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class Device {
public:
    Device(GLFWwindow* window, bool enableValidationLayers);

    ~Device();

    operator vk::Device() const
    {
        return mDevice;
    }

    vk::SurfaceKHR surface() const
    {
        return mSurface;
    }

    vk::PhysicalDevice physicalDevice() const
    {
        return mPhysicalDevice;
    }

    QueueFamilyIndices queueFamilyIndices() const
    {
        return mQueueFamilyIndices;
    }

    vk::Queue graphicsQueue() const
    {
        return mGraphicsQueue;
    }

    vk::Queue presentQueue() const
    {
        return mPresentQueue;
    }

    vk::CommandPool commandPool() const
    {
        return mCommandPool;
    }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    vk::CommandBuffer createAndBeginCommandBuffer();

    void flushAndFreeCommandBuffer(vk::CommandBuffer commandBuffer);

    vk::Format findSupportedFormat(
        const std::vector<vk::Format>& candidates,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features);

private:
    vk::Instance mInstance;
    VkDebugReportCallbackEXT mDebugCallback;
    vk::SurfaceKHR mSurface;
    vk::PhysicalDevice mPhysicalDevice;
    QueueFamilyIndices mQueueFamilyIndices;
    vk::Device mDevice;
    vk::Queue mGraphicsQueue;
    vk::Queue mPresentQueue;
    vk::CommandPool mCommandPool;
};

vk::Format findDepthAttachmentFormat(Device& device);



