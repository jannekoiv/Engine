#pragma once

#include "base.h"
#include "buffer.h"
#include "descriptor_manager.h"
#include "pipeline.h"
#include "texture.h"
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>

class Device;

// struct QuadUniform {
//     glm::highp_mat4 world_view;
//     glm::highp_mat4 proj;
// };

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
        std::vector<vk::VertexInputAttributeDescription> desc(2);
        desc[0].binding = 0;
        desc[0].location = 0;
        desc[0].format = vk::Format::eR32G32Sfloat;
        desc[0].offset = offsetof(QuadVertex, position);

        desc[1].binding = 0;
        desc[1].location = 1;
        desc[1].format = vk::Format::eA8B8G8R8UnormPack32;
        desc[1].offset = offsetof(QuadVertex, color);
        return desc;
    }

    glm::vec2 position;
    uint32_t color;
};

class Quad {
public:
    Quad(const Quad&) = delete;
    Quad(Quad&&) = default;
    ~Quad();

    Quad& operator=(const Quad&) = delete;
    Quad& operator=(Quad&&) = default;

    Quad(
        Device& device,
        // DescriptorManager& descriptor_manager,
        // TextureManager& texture_manager,
        SwapChain& swap_chain,
	Texture& depth_texture);

    // vk::DescriptorSet descriptor_set()
    // {
    //     return _descriptor_set;
    // }

    // void update_uniform_buffer();

    // QuadUniform& uniform()
    // {
    //     return _uniform;
    // }

    void draw_frame();

private:
    Device& _device;
    SwapChain& _swap_chain;
    FramebufferSet _clear_framebuffer_set;
    FramebufferSet _draw_framebuffer_set;
    std::vector<vk::CommandBuffer> _command_buffers;
    vk::Semaphore _image_available_semaphore;
    vk::Semaphore _render_finished_semaphore;
    // glm::mat4 _world_matrix;
    Buffer _vertex_buffer;
    // Buffer _uniform_buffer;
    // QuadUniform _uniform;
    // DescriptorManager& _descriptor_manager;
    // DescriptorSet _descriptor_set;
    // Pipeline _pipeline;
    vk::PipelineLayout _pipeline_layout;
    vk::Pipeline _pipeline;
};
