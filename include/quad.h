#pragma once

#include "base.h"
#include "buffer.h"
#include "descriptor_manager.h"
#include "pipeline.h"
#include "texture.h"

class Device;

struct QuadUniform {
    glm::highp_mat4 world_view;
    glm::highp_mat4 proj;
};

struct QuadVertex {
    static vk::VertexInputBindingDescription binding_description()
    {
        vk::VertexInputBindingDescription desc = {};
        desc.binding = 0;
        desc.stride = sizeof(QuadVertex);
        desc.inputRate = vk::VertexInputRate::eVertex;
        return desc;
    }

    static std::vector<vk::VertexInputAttributeDescription> attribute_description()
    {
        std::vector<vk::VertexInputAttributeDescription> desc(1);
        desc[0].binding = 0;
        desc[0].location = 0;
        desc[0].format = vk::Format::eR32G32B32A32Sfloat;
        desc[0].offset = offsetof(QuadVertex, position);
        return desc;
    }

    glm::vec2 position;
};

class Quad {
public:
    Quad(const Quad&) = delete;
    Quad(Quad&&) = default;

    Quad& operator=(const Quad&) = delete;
    Quad& operator=(Quad&&) = default;

    Quad(
        Device& device,
        DescriptorManager& descriptor_manager,
        TextureManager& texture_manager,
        SwapChain& swap_chain);

    Buffer& vertex_buffer()
    {
        return _vertex_buffer;
    }

    Pipeline& pipeline()
    {
        return _pipeline;
    }

    vk::DescriptorSet descriptor_set()
    {
        return _descriptor_set;
    }

    void update_uniform_buffer();

    QuadUniform& uniform()
    {
        return _uniform;
    }

private:
    Device& _device;
    glm::mat4 _world_matrix;
    Buffer _vertex_buffer;
    Buffer _uniform_buffer;
    QuadUniform _uniform;
    DescriptorManager& _descriptor_manager;
    DescriptorSet _descriptor_set;
    Pipeline _pipeline;
};
