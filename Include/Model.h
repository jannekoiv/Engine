#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Material.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct ModelUniform {
    glm::highp_mat4 worldView;
    glm::highp_mat4 proj;
};

struct ModelVertex {
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ModelVertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = offsetof(ModelVertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[1].offset = offsetof(ModelVertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(ModelVertex, texCoord);

        return attributeDescriptions;
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Model {
public:
    Model(const Model&) = delete;
    Model(Model&&) = default;

    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = default;

    Model(
        Device& device,
        DescriptorManager& descriptorManager,
        vk::Extent2D swapChainExtent,
        glm::mat4 worldMatrix,
        std::vector<ModelVertex> vertices,
        std::vector<uint32_t> indices,
        Material& material);

    Buffer& vertexBuffer()
    {
        return mVertexBuffer;
    }

    Buffer& indexBuffer()
    {
        return mIndexBuffer;
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

    int indexCount()
    {
        return mIndexCount;
    }

    void updateUniformBuffer();

    ModelUniform& uniform()
    {
        return mUniform;
    }

private:
    Device& mDevice;
    glm::mat4 mWorldMatrix;
    Buffer mVertexBuffer;
    Buffer mIndexBuffer;
    int mIndexCount;
    Buffer mUniformBuffer;
    ModelUniform mUniform;
    DescriptorManager& mDescriptorManager;
    DescriptorSet mDescriptorSet;
    Material mMaterial;
    Pipeline mPipeline;
};

Model createModelFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthTexture,
    std::string filename);
