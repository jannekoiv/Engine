#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct ObjectUniform {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpace;
    glm::vec3 lightDir;
};

struct Vertex {
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Object {
public:
    Object(const Object&) = delete;

    Object(Object&&) = default;

    Object(
        Device& device,
        DescriptorManager& descriptorManager,
        TextureManager& textureManager,
        DescriptorSet& sceneDescriptorSet,
        SwapChain& swapChain,
        Texture& depthTexture,
        glm::mat4 worldMatrix,
        std::vector<Vertex> vertices,
        std::vector<uint32_t> indices,
        const nlohmann::json& json,
        Texture* shadowMap,
        std::vector<glm::mat4> keyframes);

    Object& operator=(const Object&) = delete;

    Object& operator=(Object&&) = delete;

    virtual Buffer& vertexBuffer()
    {
        return mVertexBuffer;
    }

    virtual Buffer& indexBuffer()
    {
        return mIndexBuffer;
    }

    virtual Pipeline& pipeline()
    {
        return mPipeline;
    }

    virtual vk::DescriptorSet descriptorSet()
    {
        return mDescriptorSet;
    }

    virtual size_t indexCount()
    {
        return mIndices.size();
    }

    virtual void updateUniformBuffer(
        const glm::mat4& viewMatrix,
        const glm::mat4& projMatrix,
        const glm::mat4& lightSpaceMatrix,
        const glm::vec3& lightDir);

    virtual const glm::mat4& worldMatrix() const
    {
        return mUniform.world;
    }

    virtual void setWorldMatrix(const glm::mat4& worldMat)
    {
        mWorldMat = worldMat;
    }

    virtual uint32_t index(int index)
    {
        return mIndices[index];
    }

    virtual Vertex vertex(size_t index)
    {
        return mVertices[index];
    }

    virtual void setKeyframe(int iKeyframe)
    {
        mIKeyframe = iKeyframe;
    }

private:
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
    Buffer mVertexBuffer;
    Buffer mIndexBuffer;
    glm::mat4 mWorldMat;
    ObjectUniform mUniform;
    Buffer mUniformBuffer;
    DescriptorSet mDescriptorSet;
    Pipeline mPipeline;
    std::vector<glm::mat4> mKeyframes;
    int mIKeyframe;
};
