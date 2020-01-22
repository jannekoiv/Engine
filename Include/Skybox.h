#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Material.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct SkyboxUniform {
    glm::highp_mat4 worldView;
    glm::highp_mat4 proj;
};

struct SkyboxVertex {
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(SkyboxVertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(1);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = offsetof(SkyboxVertex, position);
        return attributeDescriptions;
    }

    glm::vec3 position;
};

class Skybox {
public:
    Skybox(const Skybox&) = delete;
    Skybox(Skybox&&) = default;

    Skybox& operator=(const Skybox&) = delete;
    Skybox& operator=(Skybox&&) = default;

    Skybox(
        Device& device,
        DescriptorManager& descriptorManager,
        SwapChain& swapChain,
        Texture& depthTexture);

    Buffer& vertexBuffer()
    {
        return mVertexBuffer;
    }

    Material& material()
    {
        return mMaterial;
    }

    Pipeline& pipeline()
    {
        return mPipeline;
    }

    vk::DescriptorSet descriptorSet()
    {
        return mDescriptorSet;
    }

    void updateUniformBuffer();

    SkyboxUniform& uniform()
    {
        return mUniform;
    }

private:
    Device& mDevice;
    glm::mat4 mWorldMatrix;
    Buffer mVertexBuffer;
    Buffer mUniformBuffer;
    SkyboxUniform mUniform;
    DescriptorManager& mDescriptorManager;
    DescriptorSet mDescriptorSet;
    Material mMaterial;
    Pipeline mPipeline;
};

