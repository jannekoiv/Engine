#include "../Include/Buffer.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "../Include/Texture.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

Buffer::Buffer(Buffer&& buffer)
    : mDevice(buffer.mDevice), mSize(buffer.mSize), mBuffer(buffer.mBuffer), mMemory(buffer.mMemory)
{
    buffer.mBuffer = nullptr;
    buffer.mMemory = nullptr;
}

Buffer::Buffer(
    Device& device,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags memoryProperties)
    : mDevice(device), mSize(size)
{
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive, 0, nullptr);
    mBuffer = static_cast<vk::Device>(mDevice).createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements =
        static_cast<vk::Device>(mDevice).getBufferMemoryRequirements(mBuffer);
    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        mDevice.findMemoryType(memRequirements.memoryTypeBits, memoryProperties));

    mMemory = static_cast<vk::Device>(mDevice).allocateMemory(allocInfo, nullptr);
    static_cast<vk::Device>(mDevice).bindBufferMemory(mBuffer, mMemory, 0);
}

Buffer::~Buffer()
{
    static_cast<vk::Device>(mDevice).freeMemory(mMemory);
    static_cast<vk::Device>(mDevice).destroyBuffer(mBuffer);
}

void Buffer::copy(Buffer& dstBuffer, vk::DeviceSize size)
{
    vk::CommandBuffer commandBuffer = mDevice.createAndBeginCommandBuffer();

    vk::BufferCopy region;
    region.size = size;
    commandBuffer.copyBuffer(mBuffer, dstBuffer, region);

    mDevice.flushAndFreeCommandBuffer(commandBuffer);
}

void Buffer::copy(Buffer& dstBuffer)
{
    copy(dstBuffer, dstBuffer.size());
}

void Buffer::copyToImage(Texture& dstImage, vk::Offset3D offset, vk::Extent3D extent)
{
    vk::CommandBuffer commandBuffer = mDevice.createAndBeginCommandBuffer();

    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = offset;
    region.imageExtent = extent;

    commandBuffer.copyBufferToImage(
        mBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);

    mDevice.flushAndFreeCommandBuffer(commandBuffer);
}

void Buffer::copyToImage(Texture& dstImage)
{
    copyToImage(dstImage, vk::Offset3D(0, 0, 0), dstImage.extent());
}

void* Buffer::mapMemory(vk::DeviceSize offset, vk::DeviceSize size)
{
    void* data{};
    vk::Result result =
        static_cast<vk::Device>(mDevice).mapMemory(mMemory, offset, size, {}, &data);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error{"Failed to map buffer memory"};
    }
    return data;
}
void* Buffer::mapMemory()
{
    return mapMemory(0, mSize);
}

void Buffer::unmapMemory()
{
    static_cast<vk::Device>(mDevice).unmapMemory(mMemory);
}
