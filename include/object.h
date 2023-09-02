#pragma once

#include "buffer.h"
#include "ext_includes.h"
#include "texture.h"

class Device;
class Context;
class SwapChain;

struct MeshVertex {
    static vk::VertexInputBindingDescription binding_description();
    static std::vector<vk::VertexInputAttributeDescription> attribute_description();

    glm::vec3 position;
    uint32_t color;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct MeshGeometry {
    std::vector<MeshVertex> vertices{};
    std::vector<uint32_t> indices{};
};

struct SceneUniform {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 light_dir;
};

class Object {
public:
    Object(const Object&) = delete;
    Object(Object&& rhs);
    Object(Context& context, const MeshGeometry& geometry, bool wireframe);
    ~Object();

    Object& operator=(const Object&) = delete;
    Object& operator=(Object&&) = delete;

    void draw(Context& context, SceneUniform& uniform);

    void set_world_matrix(const glm::mat4& world_matrix)
    {
        _world_matrix = world_matrix;
    }

    const glm::mat4& world_matrix() const
    {
        return _world_matrix;
    }

    void copy_vertices(const std::vector<MeshVertex>& vertices);

private:
    Context& _context;
    SwapChain& _swap_chain;

    vk::RenderPass _render_pass;
    std::vector<vk::Framebuffer> _framebuffers;

    std::vector<vk::CommandBuffer> _command_buffers;

    Buffer _vertex_buffer;
    Buffer _index_buffer;
    Buffer _uniform_buffer;

    Texture _texture;

    vk::DescriptorPool _descriptor_pool;
    vk::DescriptorSetLayout _descriptor_set_layout;
    vk::DescriptorSet _descriptor_set;

    vk::PipelineLayout _pipeline_layout;
    vk::Pipeline _pipeline;

    glm::mat4 _world_matrix;
    bool _wireframe;
};

MeshGeometry create_geometry_from_file(std::string filename);
Object create_object_from_file(Context& context, std::string filename);
