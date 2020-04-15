#pragma once

#include "../Include/Base.h"

class Device;
class SwapChain;
class Texture;

class FramebufferSet {
public:
    FramebufferSet(const FramebufferSet&) = delete;

    FramebufferSet(FramebufferSet&& rhs);

    FramebufferSet(
        Device& device,
        SwapChain& swapChain,
        Texture* depthTexture,
        const nlohmann::json& json);

    ~FramebufferSet();

    FramebufferSet& operator=(const FramebufferSet&) = delete;

    FramebufferSet& operator=(FramebufferSet&&) = delete;

    vk::RenderPass renderPass()
    {
        return mRenderPass;
    }

    vk::Framebuffer frameBuffer(int index)
    {
        return mFramebuffers[index];
    }

    size_t framebufferCount()
    {
        return mFramebuffers.size();
    }

private:
    Device& mDevice;
    vk::RenderPass mRenderPass;
    std::vector<vk::Framebuffer> mFramebuffers;
};
