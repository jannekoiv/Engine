#pragma once
#include "ext_includes.h"

class Context;

class Gui {
public:
    Gui(Context& context);
    ~Gui();

    void draw(const std::string& text);

private:
    Context& _context;
    vk::DescriptorPool _descriptor_pool;
    vk::RenderPass _render_pass;
    std::vector<vk::Framebuffer> _framebuffers;
};
