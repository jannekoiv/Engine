#pragma once
#include "../Include/Base.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Buffer.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class FramebufferSet;
class Model;
class Quad;
class Skybox;
class SwapChain;
class Texture;
class DirectionalLight;

class Renderer {
public:
    Renderer(Device& device, SwapChain& swapChain, Texture& depthTexture);
    void createCommandBuffers(std::vector<Model>& models, Skybox& skybox, Quad& quad, DirectionalLight* light);
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
