#pragma once
#include "../Include/Base.h"
#include "../Include/FramebufferSet.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class Model;
class Skybox;
class FramebufferSet;
class SwapChain;
class Texture;

class Renderer {
public:
    Renderer(Device& device, SwapChain& swapChain, Texture& depthTexture);
    void createCommandBuffers(std::vector<Model>& models, Skybox& skybox);
    void drawFrame();

private:
    Device& mDevice;
    SwapChain& mSwapChain;
    Texture& mDepthTexture;
    FramebufferSet mClearFramebufferSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
