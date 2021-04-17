#pragma once

#include "base.h"
#include "buffer.h"
#include "descriptor_manager.h"
#include "pipeline.h"
#include "texture.h"

class Device;

struct SkyboxUniform {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};

struct SkyboxVertex {
    static vk::VertexInputBindingDescription binding_description()
    {
        vk::VertexInputBindingDescription desc = {};
        desc.binding = 0;
        desc.stride = sizeof(SkyboxVertex);
        desc.inputRate = vk::VertexInputRate::eVertex;
        return desc;
    }

    static std::vector<vk::VertexInputAttributeDescription> attribute_description()
    {
        std::vector<vk::VertexInputAttributeDescription> descs(1);
        descs[0].binding = 0;
        descs[0].location = 0;
        descs[0].format = vk::Format::eR32G32B32A32Sfloat;
        descs[0].offset = offsetof(SkyboxVertex, position);
        return descs;
    }

    glm::vec3 position;
};

class Skybox {
public:
    Skybox(const Skybox&) = delete;
    Skybox(Skybox&&) = default;

    Skybox(
        Device& device,
        DescriptorManager& descriptor_manager,
        TextureManager& texture_manager,
        SwapChain& swap_chain,
        Texture& depth_texture);

    Skybox& operator=(const Skybox&) = delete;
    Skybox& operator=(Skybox&&) = default;

    Buffer& vertex_buffer()
    {
        return _vertex_buffer;
    }

    Buffer& index_buffer()
    {
        return _index_buffer;
    }

    Pipeline& pipeline()
    {
        return _pipeline;
    }

    vk::DescriptorSet descriptor_set()
    {
        return _descriptor_set;
    }

    void update_uniform_buffer(const glm::mat4& view_matrix, const glm::mat4& proj_matrix);

    const glm::mat4& world_matrix() const
    {
        return _uniform.world;
    }

    void set_world_matrix(const glm::mat4& world_matrix)
    {
        _uniform.world = world_matrix;
    }

private:
    Device& _device;
    Buffer _vertex_buffer;
    Buffer _index_buffer;
    Buffer _uniform_buffer;
    SkyboxUniform _uniform;
    DescriptorSet _descriptor_set;
    Pipeline _pipeline;
};
