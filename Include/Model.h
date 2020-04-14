#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Material.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct ModelUniform {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpace;
    glm::vec3 lightDir;
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

    Model(
        Device& device,
        DescriptorManager& descriptorManager,
        SwapChain& swapChain,
        Texture& depthTexture,
        glm::mat4 worldMatrix,
        std::vector<ModelVertex> vertices,
        std::vector<uint32_t> indices,
        Material material,
        Texture* shadowMap,
        std::vector<glm::mat4> keyframes);

    Model& operator=(const Model&) = delete;

    Model& operator=(Model&&) = delete;

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
        return mIndices.size();
    }

    void updateUniformBuffer(
        const glm::mat4& viewMatrix,
        const glm::mat4& projMatrix,
        const glm::mat4& lightSpace,
        const glm::vec3& lightDir);

    const glm::mat4& worldMatrix() const
    {
        return mUniform.world;
    }

    void setWorldMatrix(const glm::mat4& worldMatrix)
    {
        mUniform.world = worldMatrix;
    }

    uint32_t index(int index)
    {
        return mIndices[index];
    }

    ModelVertex vertex(size_t index)
    {
        return mVertices[index];
    }

    void setKeyframe(int iKeyframe)
    {
        mIKeyframe = iKeyframe;
    }

private:
    std::vector<ModelVertex> mVertices;
    std::vector<uint32_t> mIndices;
    Buffer mVertexBuffer;
    Buffer mIndexBuffer;
    ModelUniform mUniform;
    Buffer mUniformBuffer;
    DescriptorSet mDescriptorSet;
    Material mMaterial;
    Pipeline mPipeline;
    std::vector<glm::mat4> mKeyframes;
    int mIKeyframe;
};

Model createModelFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture& depthTexture,
    Texture* shadowMap,
    std::string filename);
