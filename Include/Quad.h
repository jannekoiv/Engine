#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Material.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct QuadUniform {
    glm::highp_mat4 worldView;
    glm::highp_mat4 proj;
};

struct QuadVertex {
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(QuadVertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(1);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = offsetof(QuadVertex, position);
        return attributeDescriptions;
    }

    glm::vec2 position;
};

class Quad {
public:
    Quad(const Quad&) = delete;
    Quad(Quad&&) = default;

    Quad& operator=(const Quad&) = delete;
    Quad& operator=(Quad&&) = default;

    Quad(
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

    QuadUniform& uniform()
    {
        return mUniform;
    }

private:
    Device& mDevice;
    glm::mat4 mWorldMatrix;
    Buffer mVertexBuffer;
    Buffer mUniformBuffer;
    QuadUniform mUniform;
    DescriptorManager& mDescriptorManager;
    DescriptorSet mDescriptorSet;
    Material mMaterial;
    Pipeline mPipeline;
};

;
