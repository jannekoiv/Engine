#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/Device.h"
#include "../Include/Image.h"
#include "../Include/RenderPass.h"
#include "../Include/Semaphore.h"
#include "../Include/SwapChain.h"

struct InitInfo {
    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    const bool enableValidationLayers = true;
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const unsigned width = 1920;
    const unsigned height = 1080;
};

class Engine {
public:
    Engine(const InitInfo& initInfo, GLFWwindow* window);

    ~Engine();

    Device& device()
    {
        return mDevice;
    }

    SwapChain& swapChain()
    {
        return mSwapChain;
    }

    RenderPass& renderPass()
    {
        return mRenderPass;
    }

    vk::Framebuffer frameBuffer(int index) const
    {
        return mFrameBuffers[index];
    }

    size_t frameBufferCount() const
    {
        return mFrameBuffers.size();
    }

    std::vector<vk::Framebuffer>& frameBuffers()
    {
        return mFrameBuffers;
    }

private:
    Device mDevice;
    SwapChain mSwapChain;
    RenderPass mRenderPass;
    Image mDepthImage;
    std::vector<vk::Framebuffer> mFrameBuffers;
};

