#pragma once

#include "base.h"
#include "buffer.h"
#include "descriptor_manager.h"
#include "pipeline.h"
#include "texture.h"

class Device;

struct ObjectUniform {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 light_space;
    glm::vec3 light_dir;
};

struct Vertex {
    static vk::VertexInputBindingDescription bindingDescription()
    {
        vk::VertexInputBindingDescription desc = {};
        desc.binding = 0;
        desc.stride = sizeof(Vertex);
        desc.inputRate = vk::VertexInputRate::eVertex;
        return desc;
    }

    static std::vector<vk::VertexInputAttributeDescription> attributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> descs(3);

        descs[0].binding = 0;
        descs[0].location = 0;
        descs[0].format = vk::Format::eR32G32B32A32Sfloat;
        descs[0].offset = offsetof(Vertex, position);

        descs[1].binding = 0;
        descs[1].location = 1;
        descs[1].format = vk::Format::eR32G32B32A32Sfloat;
        descs[1].offset = offsetof(Vertex, normal);

        descs[2].binding = 0;
        descs[2].location = 2;
        descs[2].format = vk::Format::eR32G32Sfloat;
        descs[2].offset = offsetof(Vertex, tex_coord);

        return descs;
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

class Object {
public:
    Object(const Object&) = delete;

    Object(Object&&) = default;

    Object(
        Device& device,
        DescriptorManager& descriptor_manager,
        TextureManager& texture_manager,
        DescriptorSet& scene_descriptor_set,
        SwapChain& swap_chain,
        Texture& depth_texture,
        glm::mat4 world_matrix,
        std::vector<Vertex> vertices,
        std::vector<uint32_t> indices,
        const nlohmann::json& json,
        Texture* shadow_map,
        std::vector<glm::mat4> keyframes);

    Object& operator=(const Object&) = delete;

    Object& operator=(Object&&) = delete;

    virtual Buffer& vertex_buffer()
    {
        return _vertex_buffer;
    }

    virtual Buffer& index_buffer()
    {
        return _index_buffer;
    }

    virtual Pipeline& pipeline()
    {
        return _pipeline;
    }

    virtual vk::DescriptorSet descriptor_set()
    {
        return _descriptor_set;
    }

    virtual size_t index_count()
    {
        return _indices.size();
    }

    virtual void update_uniform_buffer(
        const glm::mat4& view_matrix,
        const glm::mat4& proj_matrix,
        const glm::mat4& light_space_matrix,
        const glm::vec3& light_dir);

    virtual const glm::mat4& world_matrix() const
    {
        return _uniform.world;
    }

    virtual void set_world_matrix(const glm::mat4& world_matrix)
    {
        _world_matrix = world_matrix;
    }

    virtual uint32_t index(int index)
    {
        return _indices[index];
    }

    virtual Vertex vertex(size_t index)
    {
        return _vertices[index];
    }

    virtual void setKeyframe(int keyframe)
    {
        _keyframe = keyframe;
    }

private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    Buffer _vertex_buffer;
    Buffer _index_buffer;
    glm::mat4 _world_matrix;
    ObjectUniform _uniform;
    Buffer _uniform_buffer;
    DescriptorSet _descriptor_set;
    Pipeline _pipeline;
    std::vector<glm::mat4> _keyframes;
    int _keyframe;
};
