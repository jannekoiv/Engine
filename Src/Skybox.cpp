#include "../Include/Skybox.h"
#include "../Include/Device.h"
#include "../Include/TextureManager.h"
#include <fstream>
#include <iostream>

static Buffer createUniformBuffer(Device& device)
{
    return Buffer(
        device,
        sizeof(SkyboxUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

static Buffer createVertexBuffer(Device& device)
{
    std::array<SkyboxVertex, 8> vertices = {
        {{{-1.0f, 1.0f, 1.0f}},
         {{-1.0f, -1.0f, 1.0f}},
         {{1.0f, -1.0f, 1.0f}},
         {{1.0f, 1.0f, 1.0f}},
         {{-1.0f, 1.0f, -1.0f}},
         {{-1.0f, -1.0f, -1.0f}},
         {{1.0f, -1.0f, -1.0f}},
         {{1.0f, 1.0f, -1.0f}}}};

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

static Buffer createIndexBuffer(Device& device)
{
    std::array<uint32_t, 36> indices = {1, 0, 3, 1, 3, 2, 2, 3, 7, 2, 7, 6, 6, 7, 4, 6, 4, 5,
                                        5, 4, 0, 5, 0, 1, 1, 2, 6, 1, 6, 5, 0, 7, 3, 0, 4, 7};

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
    DescriptorManager& descriptorManager, vk::Buffer uniformBuffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(SkyboxUniform);

    descriptorSet.writeDescriptors({{0, 0, 1, &bufferInfo}});
    return descriptorSet;
}

Skybox::Skybox(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture& depthTexture)
    : mDevice{device},
      mVertexBuffer{createVertexBuffer(mDevice)},
      mIndexBuffer{createIndexBuffer(mDevice)},
      mUniformBuffer{createUniformBuffer(mDevice)},
      mDescriptorSet{createDescriptorSet(descriptorManager, mUniformBuffer)},
      mPipeline{
          mDevice,
          descriptorManager,
          textureManager,
          swapChain,
          &depthTexture,
          SkyboxVertex::bindingDescription(),
          SkyboxVertex::attributeDescriptions(),
          nullptr,
          mDescriptorSet.layout(),
          {{"vertexShader", "d:/Shaders/skyboxvert.spv"},
           {"fragmentShader", "d:/Shaders/skyboxfrag.spv"},
           {"texture", "d:/skybox/left.jpg"},
           {"usage", "Skybox"},
           {"depthCompareOp", "LessOrEqual"},
           {"depthTestEnable", true},
           {"depthWriteEnable", true},
           {"cullMode", "Back"}}}
{
    mUniform.world = glm::mat4{1.0f};
    std::cout << "Skybox constructed.\n";
}

void Skybox::updateUniformBuffer(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    mUniform.view = viewMatrix;
    mUniform.proj = projMatrix;
    void* data = mUniformBuffer.mapMemory();
    memcpy(data, &mUniform, sizeof(SkyboxUniform));
    mUniformBuffer.unmapMemory();
}
