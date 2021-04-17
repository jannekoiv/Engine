#include "directional_light.h"
#include "device.h"
#include "framebuffer_set.h"
#include "quad.h"
#include "skybox.h"
#include "swap_chain.h"
#include "texture.h"
#include "texture_manager.h"
#include <fstream>
#include <iostream>
#include <stb/stb_image.h>

static glm::mat4 ortho_proj_matrix()
{
    const float size_x = 20.0f;
    const float size_y = 9.f / 16.f * size_x;
    return glm::ortho(-size_x, size_x, size_y, -size_y, 1.f, 50.f);
}

static vk::CommandBuffer create_command_buffer(Device& device)
{
    vk::CommandBufferAllocateInfo command_buffer_info{
        device.command_pool(), vk::CommandBufferLevel::ePrimary, 1};
    auto command_buffer =
        static_cast<vk::Device>(device).allocateCommandBuffers(command_buffer_info).front();
    return command_buffer;
}

DirectionalLight::DirectionalLight(
    Device& device,
    DescriptorManager& descriptor_manager,
    TextureManager& texture_manager,
    SwapChain& swap_chain)
    : _device{device},
      _swap_chain{swap_chain},
      _command_buffer{create_command_buffer(_device)},
      _world_matrix{},
      _proj_matrix{ortho_proj_matrix()},
      _depth_texture{
          device,
          vk::ImageViewType::e2D,
          1,
          vk::Extent3D{swap_chain.extent().width, swap_chain.extent().height, 1},
          find_depth_attachment_optimal(device),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eSampled |
              vk::ImageUsageFlagBits::eDepthStencilAttachment,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToBorder},
      _pipeline{
          _device,
          descriptor_manager,
          texture_manager,
          swap_chain,
          &_depth_texture,
          Vertex::bindingDescription(),
          Vertex::attributeDescriptions(),
          nullptr,
          nullptr,
          {{"vertexShader", "d:/Shaders/shadowvert.spv"}, {"usage", "ShadowMap"}}}
{
    _depth_texture.transition_layout(
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::cout << "Directional light constructed.\n";
}

void DirectionalLight::drawFrame(
    std::vector<Object>& objects, vk::Extent2D swap_chain_extent)
{
    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    _command_buffer.begin(begin_info);

    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = _pipeline.framebuffer_set().render_pass();
    render_pass_info.framebuffer = _pipeline.framebuffer_set().frame_buffer(0);
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = swap_chain_extent;

    std::array<vk::ClearValue, 1> clear_values;
    clear_values[0].depthStencil = vk::ClearDepthStencilValue{1.0f, 150};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    _command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    _command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);

    for (auto& object : objects) {
        _command_buffer.bindVertexBuffers(0, {object.vertex_buffer()}, {0});
        _command_buffer.bindIndexBuffer(object.index_buffer(), 0, vk::IndexType::eUint32);

        glm::mat4 world_view_proj = _proj_matrix * view_matrix() * object.world_matrix();
        _command_buffer.pushConstants(
            _pipeline.layout(),
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(float) * 16,
            &world_view_proj);

        _command_buffer.drawIndexed(
            static_cast<uint32_t>(object.index_count()), 1, 0, 0, 0);
    }

    _command_buffer.endRenderPass();
    _command_buffer.end();

    vk::SubmitInfo submit_info(0, nullptr, nullptr, 1, &_command_buffer, 0, nullptr);
    _device.graphics_queue().submit(submit_info, nullptr);
    _device.graphics_queue().waitIdle();
    _command_buffer.reset({});
}
