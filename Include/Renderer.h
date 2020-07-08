#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
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

struct SceneUniform {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpace;
    glm::vec3 lightDir;
};

class Renderer {
public:
    Renderer(const Renderer&) = delete;

    Renderer(Renderer&&) = delete;

    Renderer(
        Device& device,
        DescriptorManager& descriptorManager,
        SwapChain& swapChain,
        Texture& depthTexture);

    ~Renderer();

    Renderer& operator=(const Renderer&) = delete;

    Renderer& operator=(Renderer&&) = delete;

    DescriptorSet& descriptorSet()
    {
        return mDescriptorSet;
    }

    void drawFrame(std::vector<Object>& objects);

    void updateUniformBuffer(
        const glm::mat4& viewMatrix,
        const glm::mat4& projMatrix,
        const glm::mat4& lightSpace,
        const glm::vec3& lightDir);

private:
    Device& mDevice;
    SwapChain& mSwapChain;
    Texture& mDepthTexture;
    FramebufferSet mClearFramebufferSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
    SceneUniform mUniform;
    Buffer mUniformBuffer;
    DescriptorSet mDescriptorSet;
};
