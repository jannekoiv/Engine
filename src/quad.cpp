#include "quad.h"
#include "device.h"
#include <fstream>
#include <iostream>

static Buffer create_uniform_buffer(Device& device)
{
    return Buffer(
        device,
        sizeof(QuadUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);
}

static Buffer create_vertex_buffer(Device& device)
{
    std::array<QuadVertex, 6> vertices = {
        {{{0.0f, 0.0f}},
         {{1.0f, 1.0f}},
         {{0.0f, 1.0f}},
         {{0.0f, 0.0f}},
         {{1.0f, 0.0f}},
         {{1.0f, 1.0f}}}};

    vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

    Buffer staging_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging_buffer.map_memory();
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    staging_buffer.unmap_memory();

    Buffer vertex_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    staging_buffer.copy(vertex_buffer);

    return vertex_buffer;
}

static DescriptorSet create_descriptor_set(
    DescriptorManager& descriptor_manager, vk::Buffer uniform_buffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

    DescriptorSet descriptor_set = descriptor_manager.create_descriptor_set(bindings);

    vk::DescriptorBufferInfo buffer_info;
    buffer_info.buffer = uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(QuadUniform);

    descriptor_set.write_descriptors({{0, 0, 1, &buffer_info}});
    return descriptor_set;
}

Quad::Quad(
    Device& device,
    DescriptorManager& descriptor_manager,
    TextureManager& texture_manager,
    SwapChain& swap_chain)
    : _device{device},
      _vertex_buffer{create_vertex_buffer(_device)},
      _uniform_buffer{create_uniform_buffer(_device)},
      _descriptor_manager{descriptor_manager},
      _descriptor_set{create_descriptor_set(_descriptor_manager, _uniform_buffer)},
      _pipeline{
          _device,
          descriptor_manager,
          texture_manager,
          swap_chain,
          nullptr,
          QuadVertex::binding_description(),
          QuadVertex::attribute_description(),
          nullptr,
          _descriptor_set.layout(),
          {{"vertexShader", "vert.spv"},
           {"fragmentShader", "frag.spv"},
           {"usage", "Quad"}}}
{
    std::cout << "Quad constructed\n";
}

void Quad::update_uniform_buffer()
{
    _uniform.world_view = glm::mat4(1.0f);
    _uniform.proj = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f);
    void* data = static_cast<vk::Device>(_device).mapMemory(
        _uniform_buffer.memory(), 0, sizeof(_uniform), {});
    memcpy(data, &_uniform, sizeof(QuadUniform));
    static_cast<vk::Device>(_device).unmapMemory(_uniform_buffer.memory());
}
