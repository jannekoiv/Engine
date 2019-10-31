#include "../Include/Image.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include <vulkan/vulkan.hpp>

Image::Image(
    Device& device,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memoryProperties)
    : mDevice(device), mExtent(extent), mFormat(format)
{
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = mExtent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = mFormat;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    mImage = static_cast<vk::Device>(mDevice).createImage(imageInfo, nullptr);

    vk::MemoryRequirements memRequirements =
        static_cast<vk::Device>(mDevice).getImageMemoryRequirements(mImage);
    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        mDevice.findMemoryType(memRequirements.memoryTypeBits, memoryProperties));

    mMemory = static_cast<vk::Device>(mDevice).allocateMemory(allocInfo, nullptr);
    static_cast<vk::Device>(mDevice).bindImageMemory(mImage, mMemory, 0);
}

Image::~Image()
{
    //    static_cast<vk::Device>(mDevice).freeMemory(mMemory);
    //    static_cast<vk::Device>(mDevice).destroyImage(mImage);
}
