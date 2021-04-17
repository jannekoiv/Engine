
#include "renderer.h"
#include "device.h"
#include "directional_light.h"
#include "framebuffer_set.h"
#include "quad.h"
#include "skybox.h"
#include "swap_chain.h"
#include "texture.h"
#include <fstream>
#include <iostream>

std::vector<vk::CommandBuffer> create_command_buffers(Device& device, SwapChain& swap_chain)
{
    vk::CommandBufferAllocateInfo command_buffer_info(
        device.command_pool(),
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(swap_chain.image_count()));

    std::vector<vk::CommandBuffer> command_buffers =
        static_cast<vk::Device>(device).allocateCommandBuffers(command_buffer_info);

    return command_buffers;
}

static DescriptorSet create_descriptor_set(
    DescriptorManager& descriptor_manager, vk::Buffer uniform_buffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0,
         vk::DescriptorType::eUniformBuffer,
         1,
         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptor_set = descriptor_manager.create_descriptor_set(bindings);

    vk::DescriptorBufferInfo info;
    info.buffer = uniform_buffer;
    info.offset = 0;
    info.range = sizeof(SceneUniform);

    std::vector<DescriptorWrite> writes = {
        {0, 0, 1, &info}}; //, {1, 0, 1, &imageInfo}};
    descriptor_set.write_descriptors(writes);
    return descriptor_set;
}

Renderer::Renderer(
    Device& device,
    DescriptorManager& descriptor_manager,
    SwapChain& swap_chain,
    Texture& depth_texture)
    : _device{device},
      _swap_chain{swap_chain},
      _depth_texture{depth_texture},
      _clear_framebuffer_set{_device, _swap_chain, &_depth_texture, {{"usage", "Clear"}}},
      _command_buffers{create_command_buffers(_device, swap_chain)},
      _image_available_semaphore{static_cast<vk::Device>(_device).createSemaphore({})},
      _render_finished_semaphore{static_cast<vk::Device>(_device).createSemaphore({})},
      _uniform{},
      _uniform_buffer{
          _device,
          sizeof(SceneUniform),
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent},
      _descriptor_set{create_descriptor_set(descriptor_manager, _uniform_buffer)}
{
    std::cout << "Renderer initialized\n";
}

Renderer::~Renderer()
{
    static_cast<vk::Device>(_device).destroySemaphore(_render_finished_semaphore);
    static_cast<vk::Device>(_device).destroySemaphore(_image_available_semaphore);
}

static void* aligned_malloc(size_t size, size_t alignment)
{
    void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc(size, alignment);
#else
    int res = posix_memalign(&data, alignment, size);
    if (res != 0)
        data = nullptr;
#endif
    return data;
}

static void aligned_free(void* data)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(data);
#else
    free(data);
#endif
}

static void clear_color(
    Device& device, SwapChain& swap_chain, int index, vk::CommandBuffer command_buffer)
{
    vk::ClearColorValue clear_color{std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f}};

    vk::ImageSubresourceRange image_range{};
    image_range.aspectMask = vk::ImageAspectFlagBits::eColor;
    image_range.baseMipLevel = 0;
    image_range.levelCount = 1;
    image_range.baseArrayLayer = 0;
    image_range.layerCount = 1;

    vk::ImageMemoryBarrier present_to_clear_barrier{};
    present_to_clear_barrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    present_to_clear_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    present_to_clear_barrier.oldLayout = vk::ImageLayout::eUndefined;
    present_to_clear_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    present_to_clear_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_to_clear_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_to_clear_barrier.image = swap_chain.image(index);
    present_to_clear_barrier.subresourceRange = image_range;

    // Change layout of image to be optimal for presenting
    vk::ImageMemoryBarrier clear_to_present_barrier{};
    clear_to_present_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    clear_to_present_barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    clear_to_present_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    clear_to_present_barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    clear_to_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clear_to_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clear_to_present_barrier.image = swap_chain.image(index);
    clear_to_present_barrier.subresourceRange = image_range;

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        {},
        {},
        {present_to_clear_barrier});

    command_buffer.clearColorImage(
        swap_chain.image(index),
        vk::ImageLayout::eTransferDstOptimal,
        clear_color,
        image_range);

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clear_to_present_barrier});
}

static void clear_depth_stencil(
    Device& device, Texture& depth_texture, vk::CommandBuffer command_buffer)
{
    vk::ClearDepthStencilValue clear_depth_stencil{1.0f, 0};

    vk::ImageSubresourceRange image_range{};
    image_range.aspectMask = vk::ImageAspectFlagBits::eDepth;
    image_range.baseMipLevel = 0;
    image_range.levelCount = 1;
    image_range.baseArrayLayer = 0;
    image_range.layerCount = 1;

    vk::ImageMemoryBarrier present_to_clear_barrier{};
    present_to_clear_barrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    present_to_clear_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    present_to_clear_barrier.oldLayout = vk::ImageLayout::eUndefined;
    present_to_clear_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    present_to_clear_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_to_clear_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_to_clear_barrier.image = depth_texture.image();
    present_to_clear_barrier.subresourceRange = image_range;

    // Change layout of image to be optimal for presenting
    vk::ImageMemoryBarrier clear_to_present_barrier{};
    clear_to_present_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    clear_to_present_barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    clear_to_present_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    clear_to_present_barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    clear_to_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clear_to_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clear_to_present_barrier.image = depth_texture.image();
    clear_to_present_barrier.subresourceRange = image_range;

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        {},
        {},
        {present_to_clear_barrier});

    command_buffer.clearDepthStencilImage(
        depth_texture.image(),
        vk::ImageLayout::eTransferDstOptimal,
        clear_depth_stencil,
        image_range);

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        {clear_to_present_barrier});
}

static void clear_pass(
    vk::CommandBuffer command_buffer,
    vk::RenderPass render_pass,
    vk::Framebuffer frame_buffer,
    vk::Extent2D swap_chain_extent)
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = frame_buffer;
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = swap_chain_extent;
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = std::array<float, 4>{0.5f, 0.4f, 0.5f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.endRenderPass();
}

float t2 = 1.0f;

static void draw_objects_pass(
    DescriptorSet& descriptor_set,
    vk::CommandBuffer command_buffer,
    int framebuffer_index,
    vk::Extent2D swap_chain_extent,
    std::vector<Object>& objects)
{
    for (auto& object : objects) {
        vk::RenderPassBeginInfo render_pass_info;
        render_pass_info.renderPass = object.pipeline().framebuffer_set().render_pass();
        render_pass_info.framebuffer =
            object.pipeline().framebuffer_set().frame_buffer(framebuffer_index);
        render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
        render_pass_info.renderArea.extent = swap_chain_extent;

        command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, object.pipeline());
        command_buffer.bindVertexBuffers(0, {object.vertex_buffer()}, {0});
        command_buffer.bindIndexBuffer(object.index_buffer(), 0, vk::IndexType::eUint32);

        command_buffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            object.pipeline().layout(),
            0,
            {descriptor_set, object.pipeline().descriptor_set(), object.descriptor_set()},
            nullptr);

        command_buffer.drawIndexed(static_cast<uint32_t>(object.index_count()), 1, 0, 0, 0);

        command_buffer.endRenderPass();
    }
}

static void draw_skybox_pass(
    vk::CommandBuffer command_buffer,
    int framebuffer_index,
    vk::Extent2D swap_chain_extent,
    Skybox& skybox)
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = skybox.pipeline().framebuffer_set().render_pass();
    render_pass_info.framebuffer =
        skybox.pipeline().framebuffer_set().frame_buffer(framebuffer_index);
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = swap_chain_extent;

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skybox.pipeline());
    command_buffer.bindVertexBuffers(0, {skybox.vertex_buffer()}, {0});
    command_buffer.bindIndexBuffer(skybox.index_buffer(), 0, vk::IndexType::eUint32);

    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        skybox.pipeline().layout(),
        0,
        {skybox.descriptor_set()},
        nullptr);

    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        skybox.pipeline().layout(),
        1,
        {skybox.pipeline().descriptor_set()},
        nullptr);

    command_buffer.drawIndexed(36, 1, 0, 0, 0);

    command_buffer.endRenderPass();
}

static void draw_quad_pass(
    vk::CommandBuffer command_buffer,
    int framebuffer_index,
    vk::Extent2D swap_chain_extent,
    Quad& quad)
{
    vk::RenderPassBeginInfo render_pass_info;
    render_pass_info.renderPass = quad.pipeline().framebuffer_set().render_pass();
    render_pass_info.framebuffer =
        quad.pipeline().framebuffer_set().frame_buffer(framebuffer_index);
    render_pass_info.renderArea.offset = vk::Offset2D(0, 0);
    render_pass_info.renderArea.extent = swap_chain_extent;

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, quad.pipeline());
    command_buffer.bindVertexBuffers(0, {quad.vertex_buffer()}, {0});

    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        quad.pipeline().layout(),
        0,
        {quad.descriptor_set()},
        nullptr);

    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        quad.pipeline().layout(),
        1,
        {quad.pipeline().descriptor_set()},
        nullptr);

    command_buffer.draw(6, 1, 0, 0);

    command_buffer.endRenderPass();
}

void Renderer::draw_frame(std::vector<Object>& objects, Quad& quad)
{
    uint32_t image_index = 0;
    static_cast<vk::Device>(_device).acquireNextImageKHR(
        _swap_chain,
        std::numeric_limits<uint64_t>::max(),
        _image_available_semaphore,
        nullptr,
        &image_index);

    vk::CommandBuffer command_buffer = _command_buffers[image_index];

    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    command_buffer.begin(begin_info);

    clear_pass(
        command_buffer,
        _clear_framebuffer_set.render_pass(),
        _clear_framebuffer_set.frame_buffer(image_index),
        _swap_chain.extent());

    draw_objects_pass(
        _descriptor_set, command_buffer, image_index, _swap_chain.extent(), objects);

    
    draw_quad_pass(command_buffer, image_index, _swap_chain.extent(), quad);

    command_buffer.end();

    vk::SubmitInfo submit_info;
    vk::PipelineStageFlags wait_stages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &_image_available_semaphore;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &_render_finished_semaphore;

    _device.graphics_queue().submit(submit_info, nullptr);

    vk::PresentInfoKHR present_info;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_finished_semaphore;
    present_info.swapchainCount = 1;
    auto swapChain = static_cast<vk::SwapchainKHR>(_swap_chain);
    present_info.pSwapchains = &swapChain;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    _device.present_queue().presentKHR(present_info);
    static_cast<vk::Device>(_device).waitIdle();
}

void Renderer::update_uniform_buffer(
    const glm::mat4& view_matrix,
    const glm::mat4& proj_matrix,
    const glm::mat4& light_space,
    const glm::vec3& light_dir)
{
    _uniform.view = view_matrix;
    _uniform.proj = proj_matrix;
    _uniform.light_space = light_space;
    _uniform.light_dir = light_dir;

    void* data = _uniform_buffer.map_memory();
    memcpy(data, &_uniform, sizeof(SceneUniform));
    _uniform_buffer.unmap_memory();
}




