#include "buffer.h"
#include "base.h"
#include "device.h"
#include "texture.h"

Buffer::Buffer(Buffer&& buffer)
    : _device(buffer._device), _size(buffer._size), _buffer(buffer._buffer), _memory(buffer._memory)
{
    buffer._buffer = nullptr;
    buffer._memory = nullptr;
}

Buffer::Buffer(
    Device& device,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags memory_properties)
    : _device(device), _size(size)
{
    vk::BufferCreateInfo buffer_info({}, size, usage, vk::SharingMode::eExclusive, 0, nullptr);
    _buffer = static_cast<vk::Device>(_device).createBuffer(buffer_info);

    vk::MemoryRequirements memory_requirements =
        static_cast<vk::Device>(_device).getBufferMemoryRequirements(_buffer);

    vk::MemoryAllocateInfo alloc_info(
        memory_requirements.size,
        _device.find_memory_type(memory_requirements.memoryTypeBits, memory_properties));

    _memory = static_cast<vk::Device>(_device).allocateMemory(alloc_info, nullptr);
    static_cast<vk::Device>(_device).bindBufferMemory(_buffer, _memory, 0);
}

Buffer::~Buffer()
{
    static_cast<vk::Device>(_device).freeMemory(_memory);
    static_cast<vk::Device>(_device).destroyBuffer(_buffer);
}

void Buffer::copy(Buffer& dst_buffer, vk::DeviceSize size)
{
    vk::CommandBuffer command_buffer = _device.create_and_begin_command_buffer();

    vk::BufferCopy region;
    region.size = size;
    command_buffer.copyBuffer(_buffer, dst_buffer, region);

    _device.flush_and_free_command_buffer(command_buffer);
}

void Buffer::copy(Buffer& dst_buffer)
{
    copy(dst_buffer, dst_buffer.size());
}

void Buffer::copy_to_texture(Texture& dst_texture, int layer, vk::Offset3D offset, vk::Extent3D extent)
{
    vk::CommandBuffer command_buffer = _device.create_and_begin_command_buffer();

    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = offset;
    region.imageExtent = extent;

    command_buffer.copyBufferToImage(
        _buffer, dst_texture.image(), vk::ImageLayout::eTransferDstOptimal, region);

    _device.flush_and_free_command_buffer(command_buffer);
}

void Buffer::copy_to_texture(Texture& dst_texture, int layer)
{
    copy_to_texture(dst_texture, layer, vk::Offset3D(0, 0, 0), dst_texture.extent());
}

void* Buffer::map_memory(vk::DeviceSize offset, vk::DeviceSize size)
{
    void* data{};
    vk::Result result =
        static_cast<vk::Device>(_device).mapMemory(_memory, offset, size, {}, &data);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error{"Failed to map buffer memory"};
    }
    return data;
}

void* Buffer::map_memory()
{
    return map_memory(0, _size);
}

void Buffer::unmap_memory()
{
    static_cast<vk::Device>(_device).unmapMemory(_memory);
}
