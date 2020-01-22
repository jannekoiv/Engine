#include "../Include/Skybox.h"
#include "../Include/Device.h"
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
    std::array<SkyboxVertex, 36> vertices = {
        {{{-1.0f, 1.0f, -1.0f}},  {{-1.0f, -1.0f, -1.0f}}, {{1.0f, -1.0f, -1.0f}},
         {{1.0f, -1.0f, -1.0f}},  {{1.0f, 1.0f, -1.0f}},   {{-1.0f, 1.0f, -1.0f}},
         {{-1.0f, -1.0f, 1.0f}},  {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, 1.0f, -1.0f}},
         {{-1.0f, 1.0f, -1.0f}},  {{-1.0f, 1.0f, 1.0f}},   {{-1.0f, -1.0f, 1.0f}},
         {{1.0f, -1.0f, -1.0f}},  {{1.0f, -1.0f, 1.0f}},   {{1.0f, 1.0f, 1.0f}},
         {{1.0f, 1.0f, 1.0f}},    {{1.0f, 1.0f, -1.0f}},   {{1.0f, -1.0f, -1.0f}},
         {{-1.0f, -1.0f, 1.0f}},  {{-1.0f, 1.0f, 1.0f}},   {{1.0f, 1.0f, 1.0f}},
         {{1.0f, 1.0f, 1.0f}},    {{1.0f, -1.0f, 1.0f}},   {{-1.0f, -1.0f, 1.0f}},
         {{-1.0f, 1.0f, -1.0f}},  {{1.0f, 1.0f, -1.0f}},   {{1.0f, 1.0f, 1.0f}},
         {{1.0f, 1.0f, 1.0f}},    {{-1.0f, 1.0f, 1.0f}},   {{-1.0f, 1.0f, -1.0f}},
         {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f, 1.0f}},  {{1.0f, -1.0f, -1.0f}},
         {{1.0f, -1.0f, -1.0f}},  {{-1.0f, -1.0f, 1.0f}},  {{1.0f, -1.0f, 1.0f}}}};

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

static Material createMaterial(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthTexture)
{
    auto texture = createCubeTextureFromFile(device, "d:/skybox/left.jpg");
    auto vertexShader = createShaderFromFile(device, "d:/Shaders/skyboxvert.spv");
    auto fragmentShader = createShaderFromFile(device, "d:/Shaders/skyboxfrag.spv");
    return Material{
        device,
        descriptorManager,
        swapChain,
        &depthTexture,
        texture,
        vertexShader,
        fragmentShader,
        MaterialUsage::Skybox};
}

Skybox::Skybox(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthTexture)
    : mDevice{device},
      mVertexBuffer{createVertexBuffer(mDevice)},
      mUniformBuffer{createUniformBuffer(mDevice)},
      mDescriptorManager{descriptorManager},
      mDescriptorSet{createDescriptorSet(mDescriptorManager, mUniformBuffer)},
      mMaterial{createMaterial(device, descriptorManager, swapChain, depthTexture)},
      mPipeline{
          mDevice,
          mMaterial,
          mDescriptorSet.layout(),
          SkyboxVertex::bindingDescription(),
          SkyboxVertex::attributeDescriptions(),
          swapChain.extent(),
          mMaterial.materialUsage()}
{
    std::cout << "Skybox initialized\n";
}

void Skybox::updateUniformBuffer()
{
    void* data = static_cast<vk::Device>(mDevice).mapMemory(
        mUniformBuffer.memory(), 0, sizeof(mUniform), {});
    memcpy(data, &mUniform, sizeof(SkyboxUniform));
    static_cast<vk::Device>(mDevice).unmapMemory(mUniformBuffer.memory());
}
