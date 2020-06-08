#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/FramebufferSet.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class DirectionalLight;
class FramebufferSet;
class Mesh;
class Object;
class Quad;
class Skybox;
class SwapChain;
class Texture;

class Renderer {
public:
    Renderer(const Renderer&) = delete;

    Renderer(Renderer&&) = delete;

    Renderer(Device& device, SwapChain& swapChain, Texture& depthTexture);

    ~Renderer();

    Renderer& operator=(const Renderer&) = delete;

    Renderer& operator=(Renderer&&) = delete;

    void drawFrame(std::vector<Object>& objects, Skybox& skybox, Quad& quad, DirectionalLight& light);

private:
    Device& mDevice;
    SwapChain& mSwapChain;
    Texture& mDepthTexture;
    FramebufferSet mClearFramebufferSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
