#pragma once

#include "../Include/Base.h"

class Device;
class SwapChain;
class Texture;

enum class MaterialUsage { Clear, Model, Skybox, Quad, ShadowMap };

class FramebufferSet {
public:
    FramebufferSet(Device& device, SwapChain& swapChain, Texture* depthTexture, MaterialUsage materialUsage);

    ~FramebufferSet();

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
    vk::RenderPass mRenderPass;
    std::vector<vk::Framebuffer> mFramebuffers;
};
