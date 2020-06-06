#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct SkyboxUniform {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
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

    Skybox(
        Device& device,
        DescriptorManager& descriptorManager,
        TextureManager& textureManager,
        SwapChain& swapChain,
        Texture& depthTexture);

    Skybox& operator=(const Skybox&) = delete;
    Skybox& operator=(Skybox&&) = default;

    Buffer& vertexBuffer()
    {
        return mVertexBuffer;
    }

    Buffer& indexBuffer()
    {
        return mIndexBuffer;
    }

    Pipeline& pipeline()
    {
        return mPipeline;
    }

    vk::DescriptorSet descriptorSet()
    {
        return mDescriptorSet;
    }

    void updateUniformBuffer(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    const glm::mat4& worldMatrix() const
    {
        return mUniform.world;
    }

    void setWorldMatrix(const glm::mat4& worldMatrix)
    {
        mUniform.world = worldMatrix;
    }

private:
    Device& mDevice;
    Buffer mVertexBuffer;
    Buffer mIndexBuffer;
    Buffer mUniformBuffer;
    SkyboxUniform mUniform;
    DescriptorSet mDescriptorSet;
    Pipeline mPipeline;
};
