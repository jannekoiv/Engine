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

class DirectionalLight {
public:
    DirectionalLight(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain);
    void drawFrame(std::vector<Model>& models, vk::Extent2D swapChainExtent);

    Texture& depthTexture()
    {
        return mDepthTexture;
    }

    const glm::mat4& projMatrix() const
    {
        return mProjMatrix;
    }

    const glm::mat4& viewMatrix() const
    {
        return mViewMatrix;
    }

    const glm::mat4 worldMatrix() const
    {
        return glm::inverse(mViewMatrix);
    }

    //private:
    Device& mDevice;
    SwapChain& mSwapChain;
    vk::CommandBuffer mCommandBuffer;
    glm::mat4 mProjMatrix;
    glm::mat4 mViewMatrix;
    Texture mDepthTexture;
    Material mMaterial;
    Pipeline mPipeline;
};
