#pragma once
#include "base.h"
#include "buffer.h"
#include "descriptor_manager.h"
#include "framebuffer_set.h"

struct UboWorldView {
    glm::mat4* world_view = nullptr;
};

class Device;
class DirectionalLight;
class FramebufferSet;
class Mesh;
class Object;
class Skybox;
class SwapChain;
class Texture;

struct SceneUniform {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 light_space;
    glm::vec3 light_dir;
};

class Renderer {
public:
    Renderer(const Renderer&) = delete;

    Renderer(Renderer&&) = delete;

    Renderer(
        Device& device,
        DescriptorManager& descriptor_manager,
        SwapChain& swap_chain,
        Texture& depth_texture);

    ~Renderer();

    Renderer& operator=(const Renderer&) = delete;

    Renderer& operator=(Renderer&&) = delete;

    DescriptorSet& descriptor_set()
    {
        return _descriptor_set;
    }

    void draw_frame(std::vector<Object>& objects);

    void update_uniform_buffer(
        const glm::mat4& view_matrix,
        const glm::mat4& proj_matrix,
        const glm::mat4& light_space,
        const glm::vec3& light_dir);

private:
    Device& _device;
    SwapChain& _swap_chain;
    Texture& _depth_texture;
    FramebufferSet _clear_framebuffer_set;
    std::vector<vk::CommandBuffer> _command_buffers;
    vk::Semaphore _image_available_semaphore;
    vk::Semaphore _render_finished_semaphore;
    SceneUniform _uniform;
    Buffer _uniform_buffer;
    DescriptorSet _descriptor_set;
};
