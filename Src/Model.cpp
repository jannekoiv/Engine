#define STB_IMAGE_IMPLEMENTATION
//#include "../Include/stb_image.h"
#include "../Include/Model.h"
#include "stb_image.h"
#include <fstream>
#include <iostream>

uint32_t readInt(std::ifstream& file)
{
    uint32_t v = 0;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

float readFloat(std::ifstream& file)
{
    float v = 0;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

std::string readString(std::ifstream& file)
{
    uint32_t len = readInt(file);
    char tmp[100];
    memset(tmp, 0, sizeof(tmp));
    file.read(tmp, len);
    return std::string(tmp);
}

void transitionImageLayout(
    Device& device,
    vk::Image image,
    vk::Format format,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = device.createAndBeginCommandBuffer();

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = {};
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        oldLayout == vk::ImageLayout::eTransferDstOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        oldLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::ePresentSrcKHR &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::ePresentSrcKHR &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        std::cout << "TRANSFERING BACK\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        oldLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::ePresentSrcKHR) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else {
        std::cout << "Unsupported layout transition.\n";
        throw std::invalid_argument("Unsupported layout transition.");
    }

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, barrier);

    device.flushAndFreeCommandBuffer(commandBuffer);
}

Image createTextureImage(Device& device, std::string filename)
{
    const int bytesPerPixel = 4;
    int width = 0;
    int height = 0;
    int channelCount = 0;
    stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channelCount, STBI_rgb_alpha);

    vk::DeviceSize imageSize = width * height * bytesPerPixel;
    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    Buffer stagingBuffer(
        device,
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = nullptr;
    static_cast<vk::Device>(device).mapMemory(stagingBuffer.memory(), 0, imageSize, {}, &data);
    memcpy(data, pixels, imageSize);
    static_cast<vk::Device>(device).unmapMemory(stagingBuffer.memory());
    stbi_image_free(pixels);

    Image image(
        device,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    transitionImageLayout(
        device,
        image,
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);

    stagingBuffer.copyToImage(image);

    transitionImageLayout(
        device,
        image,
        vk::Format::eB8G8R8A8Unorm,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    return image;
}

Model createModelFromFile(Engine& engine, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open model file!");
    }

    std::string header = readString(file);
    if (header != "paskaformaatti 1.0") {
        throw std::runtime_error("Header file not matching!");
    }

    glm::mat4 worldMatrix;
    file.read(reinterpret_cast<char*>(&worldMatrix), sizeof(glm::mat4));

    std::string material = readString(file);
    Image image = createTextureImage(engine.device(), material);

    uint32_t vertexCount = readInt(file);
    std::vector<Vertex> vertices(vertexCount);
    for (Vertex& vertex : vertices) {
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
    std::vector<uint32_t> indices(indexCount);
    for (uint32_t& index : indices) {
        index = readInt(file);
    }

    file.close();
    return Model(engine, worldMatrix, image, vertices, indices);
}

Model::Model(Engine& engine, std::string filename) : Model(createModelFromFile(engine, filename))
{
}

Buffer createUniformBuffer(Engine& engine)
{
    std::cout << "UB created\n";
    return Buffer(
        engine.device(),
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

Buffer createVertexBuffer(Device& device, std::vector<Vertex>& vertices)
{
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer(
        device,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = nullptr;
    static_cast<vk::Device>(device).mapMemory(stagingBuffer.memory(), 0, bufferSize, {}, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    static_cast<vk::Device>(device).unmapMemory(stagingBuffer.memory());

    Buffer vertexBuffer(
        device,
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    stagingBuffer.copy(vertexBuffer);

    std::cout << "VB created\n";
    return vertexBuffer;
}

Buffer createIndexBuffer(Device& device, std::vector<uint32_t>& indices)
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

    std::cout << "IB created\n";
    return indexBuffer;
}

Model::Model(
    Engine& engine,
    glm::mat4 worldMatrix,
    Image texture,
    std::vector<Vertex> vertices,
    std::vector<uint32_t> indices)
    : mEngine(engine),
      mWorldMatrix(worldMatrix),
      mTexture(texture),
      mTextureView(mTexture, vk::ImageAspectFlagBits::eColor),
      mVertexBuffer(createVertexBuffer(mEngine.device(), vertices)),
      mIndexBuffer(createIndexBuffer(mEngine.device(), indices)),
      mIndexCount(indices.size()),
      mUniformBuffer(createUniformBuffer(mEngine)),
      mTextureSampler(mEngine.device(), vk::SamplerAddressMode::eClampToEdge),
      mDescriptorPool(mEngine),
      mDescriptorSetLayout(mEngine),
      mDescriptorSet(createDescriptorSet(mUniformBuffer, mTextureView, mTextureSampler)),
      mPipeline(
          mEngine.device(),
          Vertex::getBindingDescription(),
          Vertex::getAttributeDescriptions(),
          "/home/jak/Shaders/vert.spv",
          "/home/jak/Shaders/frag.spv",
          mEngine.swapChain().extent(),
          mDescriptorSetLayout,
          mEngine.renderPass())
{
}

void Model::updateUniformBuffer()
{
    void* data = static_cast<vk::Device>(mEngine.device())
                     .mapMemory(mUniformBuffer.memory(), 0, sizeof(mUniformBufferObject), {});
    memcpy(data, &mUniformBufferObject, sizeof(UniformBufferObject));
    static_cast<vk::Device>(mEngine.device()).unmapMemory(mUniformBuffer.memory());
}

vk::DescriptorSet Model::createDescriptorSet(
    vk::Buffer uniformBuffer, vk::ImageView textureView, vk::Sampler textureSampler)
{
    std::vector<vk::DescriptorSetLayout> layouts;

    for (int i = 0; i < 1; i++) {
        layouts.push_back(mDescriptorSetLayout);
    }

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();

    vk::DescriptorSet descriptorSet =
        static_cast<vk::Device>(mEngine.device()).allocateDescriptorSets(allocInfo).front();

    std::array<vk::DescriptorBufferInfo, 15> bufferInfos;
    for (size_t i = 0; i < bufferInfos.size(); i++) {
        bufferInfos[i].buffer = uniformBuffer;
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = sizeof(UniformBufferObject);
    }

    std::array<vk::DescriptorImageInfo, 100> imageInfos;
    for (size_t i = 0; i < imageInfos.size(); i++) {
        imageInfos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfos[i].imageView = textureView;
        imageInfos[i].sampler = textureSampler;
    }

    std::array<vk::WriteDescriptorSet, 2> descriptorWrites;

    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[0].descriptorCount = bufferInfos.size();
    descriptorWrites[0].pBufferInfo = bufferInfos.data();

    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites[1].descriptorCount = imageInfos.size();
    descriptorWrites[1].pImageInfo = imageInfos.data();

    static_cast<vk::Device>(mEngine.device()).updateDescriptorSets(descriptorWrites, nullptr);

    return descriptorSet;
}
