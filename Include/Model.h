#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Image.h"
#include "../Include/Pipeline.h"
#include "../Include/RenderPass.h"
#include "../Include/Sampler.h"

class Device;

struct UniformBufferObject {
    glm::highp_mat4 worldView;
    glm::highp_mat4 proj;
};

struct Vertex {
    static vk::VertexInputBindingDescription getBindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
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

class Model {
public:
    Model(
        Device& device,
        DescriptorManager& descriptorManager,
        vk::Extent2D swapChainExtent,
        RenderPass& renderPass,
        std::string filename);

    Model(
        Device& device,
        DescriptorManager& descriptorManager,
        vk::Extent2D swapChainExtent,
        RenderPass& renderPass,
        glm::mat4 worldMatrix,
        Image texture,
        std::vector<Vertex> vertices,
        std::vector<uint32_t> indices);

    Buffer& vertexBuffer()
    {
        return mVertexBuffer;
    }

    Buffer& indexBuffer()
    {
        return mIndexBuffer;
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

    DescriptorSet createDescriptorSet(
        vk::Buffer uniformBuffer, vk::ImageView textureView, vk::Sampler textureSampler);

    //private:
    Device& mDevice;
    glm::mat4 mWorldMatrix;
    Image mTexture;
    Buffer mVertexBuffer;
    Buffer mIndexBuffer;
    int mIndexCount;
    Buffer mUniformBuffer;
    Sampler mTextureSampler;
    UniformBufferObject mUniformBufferObject;
    DescriptorManager& mDescriptorManager;
    DescriptorSet mDescriptorSet;
    Pipeline mPipeline;
};
