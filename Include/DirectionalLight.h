#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Material.h"
#include "../Include/Pipeline.h"
#include "../Include/Model.h"

class Device;
class FramebufferSet;
class Model;
class Quad;
class Skybox;
class SwapChain;
class Texture;

class DirectionalLight {
public:
    DirectionalLight(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain);
    void createCommandBuffers(std::vector<Model>& models, vk::Extent2D swapChainExtent);

    void updateUniformBuffer();
    void drawFrame();

//private:
    Device& mDevice;
    SwapChain& mSwapChain;
    vk::CommandBuffer mCommandBuffer;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
    glm::mat4 mWorldMatrix;
    Material mMaterial;
    ModelUniform mUniform;
    Buffer mUniformBuffer;
    DescriptorSet mDescriptorSet;
    Pipeline mPipeline;
};
