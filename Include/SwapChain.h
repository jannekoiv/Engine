
#pragma once

#include "../Include/Base.h"

class Device;
class ImageView;
class RenderPass;

class SwapChain {
public:
    SwapChain(Device& device);

    operator vk::SwapchainKHR() const
    {
        return mSwapChain;
    }

    vk::Format format() const
    {
        return mFormat;
    }

    const vk::Extent2D& extent() const
    {
        return mExtent;
    }

    vk::Image image(const int index)
    {
        return mImages[index];
    }

    vk::ImageView imageView(const int index)
    {
        return mImageViews[index];
    }

    size_t imageCount()
    {
        return mImageViews.size();
    }

private:
    vk::SwapchainKHR mSwapChain;
    std::vector<vk::Image> mImages;
    std::vector<vk::ImageView> mImageViews;
    vk::Format mFormat;
    vk::Extent2D mExtent;
};
