
#pragma once

#include "base.h"

class Device;
class ImageView;
class RenderPass;

class SwapChain {
public:
    SwapChain(const SwapChain&) = delete;

    SwapChain(SwapChain&&) = delete;

    SwapChain(Device& device);

    ~SwapChain();

    SwapChain& operator=(const SwapChain&) = delete;

    SwapChain& operator=(SwapChain&&) = delete;

    operator vk::SwapchainKHR() const
    {
        return _swap_chain;
    }

    vk::Format format() const
    {
        return _format;
    }

    const vk::Extent2D& extent() const
    {
        return _extent;
    }

    vk::Image image(const int index)
    {
        return _images[index];
    }

    vk::ImageView image_view(const int index)
    {
        return _image_views[index];
    }

    size_t image_count()
    {
        return _image_views.size();
    }

private:
    Device& _device;
    vk::SwapchainKHR _swap_chain;
    std::vector<vk::Image> _images;
    std::vector<vk::ImageView> _image_views;
    vk::Format _format;
    vk::Extent2D _extent;
};
