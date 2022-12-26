#pragma once

#include "device.h"
#include "ext_includes.h"
#include "swap_chain.h"
#include "texture.h"

struct Input {
    std::array<bool, GLFW_KEY_LAST> keys;
    float mouse_x;
    float mouse_y;
    float mouse_x_offset;
    float mouse_y_offset;
};

class Context {
public:
    Context(const Context&) = delete;

    Context(Context&&) = delete;

    Context(const int width, const int height);

    ~Context();

    Context& operator=(const Context&) = delete;

    Context& operator=(Context&&) = delete;

    void set_cursor_callback(GLFWcursorposfun callback);

    GLFWwindow* window()
    {
        return _window;
    }

    Device& device()
    {
        return _device;
    }

    SwapChain& swap_chain()
    {
        return _swap_chain;
    }

    Texture& depth_texture()
    {
        return _depth_texture;
    }

    vk::CommandPool command_pool() const
    {
        return _command_pool;
    }

    uint32_t image_index()
    {
        return _image_index;
    }

    vk::CommandBuffer command_buffer()
    {
        return _command_buffers[_image_index];
    }

    void begin_frame();

    void end_frame();

    void clear_frame();

    const Input& input();

    void hide_cursor(bool hide);

    void send_keys(std::array<bool, GLFW_KEY_LAST>& keys);

private:
    GLFWwindow* _window;
    Device _device;
    SwapChain _swap_chain;
    Texture _depth_texture;
    vk::CommandPool _command_pool;
    std::vector<vk::CommandBuffer> _command_buffers;
    vk::Semaphore _image_available_semaphore;
    vk::Semaphore _render_finished_semaphore;
    uint32_t _image_index;
    vk::RenderPass _render_pass;
    std::vector<vk::Framebuffer> _framebuffers;
    Input _input;
};
