#pragma once

#include "../Include/Base.h"
#include "../Include/RenderPass.h"

class Device;

class Framebuffer {
public:
    Framebuffer(
        Device& device,
        vk::ImageView imageView,
        vk::ImageView depthImageView,
        vk::Extent2D swapChainExtent,
        RenderPass& renderPass);

    ~Framebuffer();

    operator vk::Framebuffer() const
    {
        return mFramebuffer;
    }

    RenderPass& renderPass()
    {
        return mRenderPass;
    }

private:
    Device& mDevice;
    RenderPass& mRenderPass;
    vk::Framebuffer mFramebuffer;
};
