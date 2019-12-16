
#pragma once

#include "../Include/Base.h"

class Device;

class RenderPass {
public:
    RenderPass(Device& device, vk::Format swapChainFormat, vk::AttachmentLoadOp loadOp);

    ~RenderPass();

    operator vk::RenderPass() const
    {
        return mRenderPass;
    }

    //vk::Format depthAttachmentFormat() const
    //{
    //    return mDepthAttachmentFormat;
    //}
private:
    Device& mDevice;
    vk::Format mDepthAttachmentFormat;
    vk::RenderPass mRenderPass;
};
