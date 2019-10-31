#pragma once

#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Sampler.h"
#include "../Include/Base.h"
#include "../Include/DescriptorPool.h"
#include "../Include/DescriptorSetLayout.h"
#include "../Include/Pipeline.h"
#include "../Include/Engine.h"

struct UniformBufferObject {
    glm::highp_mat4 worldView;
    glm::highp_mat4 proj;
};

class Model {
public:
    Model(Engine& engine, std::string filename);

    Model(
        Engine& engine,
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

    vk::DescriptorSet createDescriptorSet(
        vk::Buffer uniformBuffer, vk::ImageView textureView, vk::Sampler textureSampler);

    //private:
    Engine& mEngine;
    glm::mat4 mWorldMatrix;
    Image mTexture;
    Buffer mVertexBuffer;
    Buffer mIndexBuffer;
    int mIndexCount;
    Buffer mUniformBuffer;
    Sampler mTextureSampler;
    UniformBufferObject mUniformBufferObject;
    DescriptorPool mDescriptorPool;
    DescriptorSetLayout mDescriptorSetLayout;
    vk::DescriptorSet mDescriptorSet;
    Pipeline mPipeline;
};
