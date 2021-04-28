#include "object.h"
#include "device.h"

static Buffer create_vertex_buffer(Device& device, std::vector<Vertex>& vertices)
{
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

static Buffer create_index_buffer(Device& device, std::vector<uint32_t>& indices)
{
    vk::DeviceSize size = sizeof(indices[0]) * indices.size();

    Buffer staging_buffer(
        device,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = nullptr;
    static_cast<vk::Device>(device).mapMemory(
        staging_buffer.memory(), 0, size, {}, &data);
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
    DescriptorManager& descriptor_manager, vk::Buffer uniform_buffer, Texture* shadow_map)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0,
         vk::DescriptorType::eUniformBuffer,
         1,
         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment}};
    //{1,
    // vk::DescriptorType::eCombinedImageSampler,
    // 1,
    // vk::ShaderStageFlagBits::eFragment}};

    DescriptorSet descriptor_set = descriptor_manager.create_descriptor_set(bindings);

    vk::DescriptorBufferInfo buffer_info;
    buffer_info.buffer = uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(ObjectUniform);

    //vk::DescriptorImageInfo imageInfo{};
    //imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    //imageInfo.imageView = shadowMap->imageView();
    //imageInfo.sampler = shadowMap->sampler();

    std::vector<DescriptorWrite> writes = {
        {0, 0, 1, &buffer_info}}; //, {1, 0, 1, &imageInfo}};
    descriptor_set.write_descriptors(writes);
    return descriptor_set;
}

Object::Object(
    Device& device,
    DescriptorManager& descriptor_manager,
    TextureManager& texture_manager,
    DescriptorSet& scene_descriptor_set,
    SwapChain& swap_chain,
    Texture& depth_texture,
    glm::mat4 world_matrix,
    std::vector<Vertex> vertices,
    std::vector<uint32_t> indices,
    const nlohmann::json& json,
    Texture* shadow_map,
    std::vector<glm::mat4> keyframes)
    : _vertices{vertices},
      _indices{indices},
      _vertex_buffer(create_vertex_buffer(device, _vertices)),
      _index_buffer(create_index_buffer(device, _indices)),
      _world_matrix{world_matrix},
      _uniform{},
      _uniform_buffer{
          device,
          sizeof(ObjectUniform),
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent},
      _descriptor_set(create_descriptor_set(descriptor_manager, _uniform_buffer, shadow_map)),
      _pipeline{
          device,
          descriptor_manager,
          texture_manager,
          swap_chain,
          &depth_texture,
          Vertex::bindingDescription(),
          Vertex::attributeDescriptions(),
          scene_descriptor_set.layout(),
          _descriptor_set.layout(),
          json},
      _keyframes{keyframes},
      _keyframe{-1}
{
    std::cout << "Object constructor\n";
}

void Object::update_uniform_buffer(
    const glm::mat4& view_matrix,
    const glm::mat4& proj_matrix,
    const glm::mat4& light_space_matrix,
    const glm::vec3& light_dir)
{
    if (_keyframe >= 0) {
        _uniform.world = _keyframes[_keyframe];
    } else {
        _uniform.world = _world_matrix;
    }
    _uniform.view = view_matrix;
    _uniform.proj = proj_matrix;
    _uniform.light_space = light_space_matrix;
    _uniform.light_dir = light_dir;

    void* data = _uniform_buffer.map_memory();
    memcpy(data, &_uniform, sizeof(ObjectUniform));
    _uniform_buffer.unmap_memory();
}
