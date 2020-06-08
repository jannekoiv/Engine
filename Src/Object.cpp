#include "../Include/Object.h"
#include "../Include/Device.h"
#include <fstream>
#include <iostream>

static Buffer createVertexBuffer(Device& device, std::vector<Vertex>& vertices)
{
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    Buffer stagingBuffer(
        device,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = stagingBuffer.mapMemory();
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    stagingBuffer.unmapMemory();

    Buffer vertexBuffer(
        device,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    stagingBuffer.copy(vertexBuffer);

    return vertexBuffer;
}

static Buffer createIndexBuffer(Device& device, std::vector<uint32_t>& indices)
{
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingBuffer(
        device,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = nullptr;
    static_cast<vk::Device>(device).mapMemory(stagingBuffer.memory(), 0, bufferSize, {}, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    static_cast<vk::Device>(device).unmapMemory(stagingBuffer.memory());

    Buffer indexBuffer(
        device,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    stagingBuffer.copy(indexBuffer);

    return indexBuffer;
}

static DescriptorSet createDescriptorSet(
    DescriptorManager& descriptorManager, vk::Buffer uniformBuffer, Texture* shadowMap)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0,
         vk::DescriptorType::eUniformBuffer,
         1,
         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},
        {1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(Uniform);

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = shadowMap->imageView();
    imageInfo.sampler = shadowMap->sampler();

    std::vector<DescriptorWrite> writes = {{0, 0, 1, &bufferInfo}, {1, 0, 1, &imageInfo}};
    descriptorSet.writeDescriptors(writes);
    return descriptorSet;
}

Object::Object(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture& depthTexture,
    glm::mat4 worldMatrix,
    std::vector<Vertex> vertices,
    std::vector<uint32_t> indices,
    const nlohmann::json& json,
    Texture* shadowMap,
    std::vector<glm::mat4> keyframes)
    : mVertices{vertices},
      mIndices{indices},
      mVertexBuffer(createVertexBuffer(device, mVertices)),
      mIndexBuffer(createIndexBuffer(device, mIndices)),
      mUniform{},
      mUniformBuffer{
          device,
          sizeof(Uniform),
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent},
      mDescriptorSet(createDescriptorSet(descriptorManager, mUniformBuffer, shadowMap)),
      mPipeline{
          device,
          descriptorManager,
          textureManager,
          swapChain,
          &depthTexture,
          Vertex::bindingDescription(),
          Vertex::attributeDescriptions(),
          mDescriptorSet.layout(),
          json},
      mKeyframes{keyframes},
      mIKeyframe{-1}
{
    mUniform.world = worldMatrix;
    std::cout << "Object constructor\n";
}

void Object::updateUniformBuffer(
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix,
    const glm::mat4& lightSpace,
    const glm::vec3& lightDir)
{
    if (mIKeyframe >= 0) {
        mUniform.world = mKeyframes[mIKeyframe];
    }
    mUniform.view = viewMatrix;
    mUniform.proj = projMatrix;
    mUniform.lightSpace = lightSpace;
    mUniform.lightDir = lightDir;

    void* data = mUniformBuffer.mapMemory();
    memcpy(data, &mUniform, sizeof(Uniform));
    mUniformBuffer.unmapMemory();
}
