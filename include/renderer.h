#pragma once

#include "ext_includes.h"

class Context;
class Device;
class FramebufferSet;
class Object;
class SwapChain;
class Texture;

class Renderer {
public:
    Renderer(const Renderer&) = delete;

    Renderer(Renderer&&) = delete;

    Renderer(Context& context);

    ~Renderer();

    Renderer& operator=(const Renderer&) = delete;

    Renderer& operator=(Renderer&&) = delete;

    void clear();

    void update_uniform_buffer(
        const glm::mat4& view_matrix,
        const glm::mat4& proj_matrix,
        const glm::mat4& light_space,
        const glm::vec3& light_dir);

private:
    Context& _context;
    vk::RenderPass _render_pass;
    std::vector<vk::Framebuffer> _framebuffers;
};
