#include "../Include/Model.h"
#include "../Include/Device.h"
#include <fstream>
#include <iostream>

static Buffer createUniformBuffer(Device& device)
{
    return Buffer(
        device,
        sizeof(ModelUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

static Buffer createVertexBuffer(Device& device, std::vector<ModelVertex>& vertices)
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
    bufferInfo.range = sizeof(ModelUniform);

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = shadowMap->view();
    imageInfo.sampler = shadowMap->sampler();

    std::vector<DescriptorWrite> writes = {{0, 0, 1, &bufferInfo}, {1, 0, 1, &imageInfo}};
    descriptorSet.writeDescriptors(writes);
    return descriptorSet;
}

Model::Model(
    Device& device,
    DescriptorManager& descriptorManager,
    vk::Extent2D swapChainExtent,
    glm::mat4 worldMatrix,
    std::vector<ModelVertex> vertices,
    std::vector<uint32_t> indices,
    Material& material,
    Texture* shadowMap,
    std::vector<glm::mat4> keyframes)
    : mDevice(device),
      mVertexBuffer(createVertexBuffer(mDevice, vertices)),
      mIndexBuffer(createIndexBuffer(mDevice, indices)),
      mVertices{vertices},
      mIndexCount(indices.size()),
      mIndices{indices},
      mUniform{},
      mUniformBuffer(createUniformBuffer(mDevice)),
      mDescriptorManager(descriptorManager),
      mDescriptorSet(createDescriptorSet(mDescriptorManager, mUniformBuffer, shadowMap)),
      mMaterial{std::move(material)},
      mPipeline{
          mDevice,
          mMaterial,
          mDescriptorSet.layout(),
          ModelVertex::bindingDescription(),
          ModelVertex::attributeDescriptions(),
          swapChainExtent,
          mMaterial.materialUsage()},
      mKeyframes{keyframes},
      mIKeyframe{-1}
{
    mUniform.world = worldMatrix;
    std::cout << "Model constructor\n";
}

void Model::updateUniformBuffer(
    const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::mat4& lightSpace, const glm::vec3& lightDir)
{
    if (mIKeyframe >= 0) {
        mUniform.world = mKeyframes[mIKeyframe];
    }
    mUniform.view = viewMatrix;
    mUniform.proj = projMatrix;
    mUniform.lightSpace = lightSpace;
    mUniform.lightDir = lightDir;

    void* data = mUniformBuffer.mapMemory();
    memcpy(data, &mUniform, sizeof(ModelUniform));
    mUniformBuffer.unmapMemory();
}

Model createModelFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    SwapChain& swapChain,
    Texture& depthTexture,
    Texture* shadowMap,
    std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open model file!");
    }

    std::string header = readString(file);
    if (header != "paskaformaatti 1.0") {
        std::cout << "HEADER\n";
        throw std::runtime_error("Header file not matching!");
    }

    glm::mat4 worldMatrix;
    file.read(reinterpret_cast<char*>(&worldMatrix), sizeof(glm::mat4));

    uint32_t vertexCount = readInt(file);
    std::cout << "vertex count " << vertexCount << std::endl;
    std::vector<ModelVertex> vertices(vertexCount);
    for (ModelVertex& vertex : vertices) {
        vertex.position.x = readFloat(file);
        vertex.position.y = readFloat(file);
        vertex.position.z = readFloat(file);
        vertex.normal.x = readFloat(file);
        vertex.normal.y = readFloat(file);
        vertex.normal.z = readFloat(file);
        vertex.texCoord.x = readFloat(file);
        vertex.texCoord.y = readFloat(file);
    }

    uint32_t indexCount = readInt(file);
    std::cout << "index count " << indexCount << std::endl;
    std::vector<uint32_t> indices(indexCount);
    for (uint32_t& index : indices) {
        index = readInt(file);
    }

    auto materialFilename = readString(file);
    Material material = createMaterialFromFile(
        device, descriptorManager, swapChain, &depthTexture, materialFilename, MaterialUsage::Model);

    uint32_t keyframeCount = readInt(file);
    std::cout << "keyframeCount " << keyframeCount << "\n";
    std::vector<glm::mat4> keyframes(keyframeCount);
    for (int i = 0; i < keyframeCount; ++i) {
        file.read(reinterpret_cast<char*>(&keyframes[i]), sizeof(glm::mat4));
        keyframes[i][3][0] *= 0.001f;
        keyframes[i][3][1] *= 0.001f;
        keyframes[i][3][2] *= 0.001f;
    }

    file.close();
    return Model{
        device, descriptorManager, swapChain.extent(), worldMatrix, vertices, indices, material, shadowMap, keyframes};
}
