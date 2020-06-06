#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/FramebufferSet.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class FramebufferSet;
class Mesh;
class Quad;
class Skybox;
class SwapChain;
class Texture;
class DirectionalLight;

class Renderer {
public:
    Renderer(const Renderer&) = delete;

    Renderer(Renderer&&) = delete;

    Renderer(Device& device, SwapChain& swapChain, Texture& depthTexture);

    ~Renderer();

    Renderer& operator=(const Renderer&) = delete;

    Renderer& operator=(Renderer&&) = delete;

    void drawFrame(std::vector<Mesh>& models, Skybox& skybox, Quad& quad, DirectionalLight& light);

private:
    Device& mDevice;
    SwapChain& mSwapChain;
    Texture& mDepthTexture;
    FramebufferSet mClearFramebufferSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
