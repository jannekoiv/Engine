#pragma once

#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Object.h"
#include "../Include/Pipeline.h"
#include "../Include/Texture.h"

class Device;

struct MeshUniform {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpace;
    glm::vec3 lightDir;
};

struct MeshVertex {
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(MeshVertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = offsetof(MeshVertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[1].offset = offsetof(MeshVertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(MeshVertex, texCoord);

        return attributeDescriptions;
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Mesh : public Object {
public:
    Mesh(const Mesh&) = delete;

    Mesh(Mesh&&) = default;

    Mesh(
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
        std::vector<glm::mat4> keyframes);

    Mesh& operator=(const Mesh&) = delete;

    Mesh& operator=(Mesh&&) = delete;

private:
};

Mesh createMeshFromFile(
    Device& device,
    DescriptorManager& descriptorManager,
    TextureManager& textureManager,
    SwapChain& swapChain,
    Texture& depthTexture,
    Texture* shadowMap,
    std::string filename);
