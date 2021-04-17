#pragma once

#include "base.h"
#include <nlohmann/json.hpp>

class Device;
class SwapChain;
class Texture;

class FramebufferSet {
public:
    FramebufferSet(const FramebufferSet&) = delete;

    FramebufferSet(FramebufferSet&& rhs);

    FramebufferSet(
        Device& device,
        SwapChain& swap_chain,
        Texture* depth_texture,
        const nlohmann::json& json);

    ~FramebufferSet();

    FramebufferSet& operator=(const FramebufferSet&) = delete;

    FramebufferSet& operator=(FramebufferSet&&) = delete;

    vk::RenderPass render_pass()
    {
        return _render_pass;
    }

    vk::Framebuffer frame_buffer(int index)
    {
        return _frame_buffers[index];
    }

    size_t frame_buffer_count()
    {
        return _frame_buffers.size();
    }

private:
    Device& _device;
    vk::RenderPass _render_pass;
    std::vector<vk::Framebuffer> _frame_buffers;
};
