#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/FramebufferSet.h"
#include "../Include/Material.h"
#include "../Include/Model.h"
#include "../Include/Pipeline.h"

class Device;
class FramebufferSet;
class Model;
class Quad;
class Skybox;
class SwapChain;
class Texture;

struct LightUniform {
    glm::highp_mat4 world;
    glm::highp_mat4 view;
    glm::highp_mat4 proj;
};

class DirectionalLight {
public:
    DirectionalLight(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain);
    void createCommandBuffers(std::vector<Model>& models, vk::Extent2D swapChainExtent);
    void updateUniformBuffer();
    void drawFrame();

    Texture& depthTexture()
    {
        return mDepthTexture;
    }

    //private:
    Device& mDevice;
    SwapChain& mSwapChain;
    vk::CommandBuffer mCommandBuffer;
    glm::mat4 mViewMatrix;
    Texture mDepthTexture;
    Material mMaterial;
    LightUniform mUniform;
    Buffer mUniformBuffer;
    DescriptorSet mDescriptorSet;
    Pipeline mPipeline;
};
