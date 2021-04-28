#include "skybox.h"
#include "device.h"
#include "texture_manager.h"


static Buffer create_uniform_buffer(Device& device)
{
    return Buffer(
        device,
        sizeof(SkyboxUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

static Buffer create_vertex_buffer(Device& device)
{
    std::array<SkyboxVertex, 8> vertices = {
        {{{-1.0f, 1.0f, 1.0f}},
         {{-1.0f, -1.0f, 1.0f}},
         {{1.0f, -1.0f, 1.0f}},
         {{1.0f, 1.0f, 1.0f}},
         {{-1.0f, 1.0f, -1.0f}},
         {{-1.0f, -1.0f, -1.0f}},
         {{1.0f, -1.0f, -1.0f}},
         {{1.0f, 1.0f, -1.0f}}}};

    vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

    Buffer staging_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

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

static Buffer create_index_buffer(Device& device)
{
    std::array<uint32_t, 36> indices = {1, 0, 3, 1, 3, 2, 2, 3, 7, 2, 7, 6, 6, 7, 4, 6, 4, 5,
                                        5, 4, 0, 5, 0, 1, 1, 2, 6, 1, 6, 5, 0, 7, 3, 0, 4, 7};

    vk::DeviceSize size = sizeof(indices[0]) * indices.size();

    Buffer staging_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = nullptr;
    static_cast<vk::Device>(device).mapMemory(staging_buffer.memory(), 0, size, {}, &data);
    memcpy(data, indices.data(), (size_t)size);
    static_cast<vk::Device>(device).unmapMemory(staging_buffer.memory());

    Buffer index_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    staging_buffer.copy(index_buffer);
    return index_buffer;
}

static DescriptorSet create_descriptor_set(
    DescriptorManager& descriptor_manager, vk::Buffer uniform_buffer)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}};

    DescriptorSet descriptor_set = descriptor_manager.create_descriptor_set(bindings);

    vk::DescriptorBufferInfo info;
    info.buffer = uniform_buffer;
    info.offset = 0;
    info.range = sizeof(SkyboxUniform);

    descriptor_set.write_descriptors({{0, 0, 1, &info}});
    return descriptor_set;
}

Skybox::Skybox(
    Device& device,
    DescriptorManager& descriptor_manager,
    TextureManager& texture_manager,
    SwapChain& swap_chain,
    Texture& depth_texture)
    : _device{device},
      _vertex_buffer{create_vertex_buffer(_device)},
      _index_buffer{create_index_buffer(_device)},
      _uniform_buffer{create_uniform_buffer(_device)},
      _descriptor_set{create_descriptor_set(descriptor_manager, _uniform_buffer)},
      _pipeline{
          _device,
          descriptor_manager,
          texture_manager,
          swap_chain,
          &depth_texture,
          SkyboxVertex::binding_description(),
          SkyboxVertex::attribute_description(),
          nullptr,
          _descriptor_set.layout(),
          {{"vertexShader", "d:/Shaders/skyboxvert.spv"},
           {"fragmentShader", "d:/Shaders/skyboxfrag.spv"},
           {"texture", "d:/skybox/left.jpg"},
           {"usage", "Skybox"},
           {"depthCompareOp", "LessOrEqual"},
           {"depthTestEnable", true},
           {"depthWriteEnable", true},
           {"cullMode", "Back"}}}
{
    _uniform.world = glm::mat4{1.0f};
    std::cout << "Skybox constructed.\n";
}

void Skybox::update_uniform_buffer(const glm::mat4& view_matrix, const glm::mat4& proj_matrix)
{
    _uniform.view = view_matrix;
    _uniform.proj = proj_matrix;
    void* data = _uniform_buffer.map_memory();
    memcpy(data, &_uniform, sizeof(SkyboxUniform));
    _uniform_buffer.unmap_memory();
}
