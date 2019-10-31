
#pragma once

#include "../Include/Base.h"

class Device;
class InitInfo;
class ImageView;
class RenderPass;

class SwapChain {
public:
    SwapChain(Device& device, const vk::Extent2D& extent);

    operator const vk::SwapchainKHR() const
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

    std::vector<ImageView>& imageViews()
    {
        return mImageViews;
    }

private:
    Device& mDevice;
    vk::SwapchainKHR mSwapChain;
    std::vector<ImageView> mImageViews;
    vk::Format mFormat;
    vk::Extent2D mExtent;
};
