#include "../Include/Quad.h"
#include "../Include/Device.h"
#include <fstream>
#include <iostream>

static Buffer createUniformBuffer(Device& device)
{
    return Buffer(
        device,
        sizeof(QuadUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

static Buffer createVertexBuffer(Device& device)
{
    std::array<QuadVertex, 6> vertices = {
        {{{0.0f, 0.0f}}, {{1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f}}, {{1.0f, 0.0f}}, {{1.0f, 1.0f}}}};

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

static DescriptorSet createDescriptorSet(DescriptorManager& descriptorManager, vk::Buffer uniformBuffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

    DescriptorSet descriptorSet = descriptorManager.createDescriptorSet(bindings);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(QuadUniform);

    descriptorSet.writeDescriptors({{0, 0, 1, &bufferInfo}});
    return descriptorSet;
}

static Material createMaterial(
    Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain, Texture& texture)
{
    std::cout << "Shadowmap in quad createMaterial " << texture.imageView() << "\n";

    std::cout << "creating texture\n";

    auto vertexShader = createShaderFromFile(device, "d:/Shaders/quadvert.spv");
    auto fragmentShader = createShaderFromFile(device, "d:/Shaders/quadfrag.spv");

    std::cout << "creating material\n";

    return Material{
        device,
        descriptorManager,
        swapChain,
        nullptr,
        &texture,
        vertexShader,
        fragmentShader,
        MaterialUsage::Quad};
}

Quad::Quad(Device& device, DescriptorManager& descriptorManager, SwapChain& swapChain, Texture& texture)
    : mDevice{device},
      mVertexBuffer{createVertexBuffer(mDevice)},
      mUniformBuffer{createUniformBuffer(mDevice)},
      mDescriptorManager{descriptorManager},
      mDescriptorSet{createDescriptorSet(mDescriptorManager, mUniformBuffer)},
      mMaterial{createMaterial(device, descriptorManager, swapChain, texture)},
      mPipeline{
          mDevice,
          mMaterial,
          mDescriptorSet.layout(),
          QuadVertex::bindingDescription(),
          QuadVertex::attributeDescriptions(),
          swapChain.extent(),
          mMaterial.materialUsage()}
{
    std::cout << "Quad constructed\n";
}

void Quad::updateUniformBuffer()
{
    mUniform.worldView = glm::mat4(1.0f);
    mUniform.proj = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f);
    void* data = static_cast<vk::Device>(mDevice).mapMemory(mUniformBuffer.memory(), 0, sizeof(mUniform), {});
    memcpy(data, &mUniform, sizeof(QuadUniform));
    static_cast<vk::Device>(mDevice).unmapMemory(mUniformBuffer.memory());
}