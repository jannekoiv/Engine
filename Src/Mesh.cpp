#include "../Include/Mesh.h"
#include "../Include/Device.h"
#include <fstream>
#include <iostream>

Mesh::Mesh(
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
    : Object{
          device,
          descriptorManager,
          textureManager,
          swapChain,
          depthTexture,
          worldMatrix,
          vertices,
          indices,
          json,
          shadowMap,
          keyframes}
{
}

Mesh createMeshFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture& depthTexture,
    Texture* shadowMap,
    std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open mesh file!");
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
    //std::vector<ModelVertex> vertices(vertexCount);
    //for (ModelVertex& vertex : vertices) {
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
    std::cout << "index count " << indexCount << std::endl;
    std::vector<uint32_t> indices(indexCount);
    for (uint32_t& index : indices) {
        index = readInt(file);
    }

    auto materialFilename = readString(file);
    using Json = nlohmann::json;
    std::ifstream materialFile{materialFilename};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open material file!");
    }
    Json json;
    materialFile >> json;

    auto keyframeCount = readInt(file);
    std::cout << "keyframeCount " << keyframeCount << "\n";
    std::vector<glm::mat4> keyframes(keyframeCount);
    for (auto i = 0; i < keyframeCount; ++i) {
        file.read(reinterpret_cast<char*>(&keyframes[i]), sizeof(glm::mat4));
        keyframes[i][3][0] *= 0.001f;
        keyframes[i][3][1] *= 0.001f;
        keyframes[i][3][2] *= 0.001f;
    }

    file.close();
    return Mesh{
        device,
        descriptorManager,
        textureManager,
        swapChain,
        depthTexture,
        worldMatrix,
        vertices,
        indices,
        json,
        shadowMap,
        keyframes};
}
