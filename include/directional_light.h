#pragma once
#include "base.h"
#include "buffer.h"
#include "descriptor_manager.h"
#include "framebuffer_set.h"
#include "object.h"
#include "pipeline.h"

class Device;
class FramebufferSet;
class Mesh;
class Quad;
class Skybox;
class SwapChain;
class Texture;

class DirectionalLight {
public:
    DirectionalLight(
        Device& device,
        DescriptorManager& descriptor_manager,
        TextureManager& texture_manager,
        SwapChain& swap_chain);

    void drawFrame(std::vector<Object>& objects, vk::Extent2D swap_chain_extent);

    Texture& depth_texture()
    {
        return _depth_texture;
    }

    const glm::mat4& proj_matrix() const
    {
        return _proj_matrix;
    }

    glm::mat4 view_matrix() const
    {
        return glm::inverse(_world_matrix);
    }

    const glm::mat4& world_matrix() const
    {
        return _world_matrix;
    }

    void set_world_matrix(const glm::mat4& world_matrix)
    {
        _world_matrix = world_matrix;
    }

    //private:
    Device& _device;
    SwapChain& _swap_chain;
    vk::CommandBuffer _command_buffer;
    glm::mat4 _world_matrix;
    glm::mat4 _proj_matrix;
    Texture _depth_texture;
    Pipeline _pipeline;
};
