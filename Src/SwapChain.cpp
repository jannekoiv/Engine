#include "../Include/SwapChain.h"
#include "../Include/Device.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stddef.h>
#include <vector>
#include <vulkan/vulkan.hpp>

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
        vk::SurfaceFormatKHR format;
        format.format = vk::Format::eB8G8R8A8Unorm;
        format.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        return format;
    }
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes)
{
    return vk::PresentModeKHR::eFifo;
    vk::PresentModeKHR preferredMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        } else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
            preferredMode = availablePresentMode;
        }
    }
    return preferredMode;
}

uint32_t chooseSwapImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    return imageCount;
}

static vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView = device.createImageView(viewInfo, nullptr);
    return imageView;
}

SwapChain::SwapChain(Device& device) : mDevice{device}
{
    SwapChainSupportDetails swapChainSupportDetails(mDevice.surface(), mDevice.physicalDevice());
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
    mFormat = surfaceFormat.format;
    mExtent = swapChainSupportDetails.capabilities.currentExtent;
    auto presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
    auto imageCount = chooseSwapImageCount(swapChainSupportDetails.capabilities);

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = mDevice.surface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = mExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    int graphicsFamily = mDevice.queueFamilyIndices().graphics;
    int presentFamily = mDevice.queueFamilyIndices().present;
    if (graphicsFamily != presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t indices[] = {static_cast<uint32_t>(graphicsFamily), static_cast<uint32_t>(presentFamily)};
        createInfo.pQueueFamilyIndices = indices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    mSwapChain = static_cast<vk::Device>(mDevice).createSwapchainKHR(createInfo, nullptr);
    auto images = static_cast<vk::Device>(mDevice).getSwapchainImagesKHR(mSwapChain);
    std::cout << "swapchain images size " << images.size() << "\n";

    for (size_t i = 0; i < images.size(); i++) {
        mImages.push_back(images[i]);
        mImageViews.push_back(createImageView(mDevice, images[i], mFormat));
    }
}

SwapChain::~SwapChain()
{
    for (auto imageView : mImageViews) {
        static_cast<vk::Device>(mDevice).destroyImageView(imageView);
    }
    static_cast<vk::Device>(mDevice).destroySwapchainKHR(mSwapChain);
}

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D& extent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        std::cout << "FORCED\n";
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = extent;
        actualExtent.width = std::max(
            capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}
