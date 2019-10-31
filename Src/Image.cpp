#include "../Include/Image.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include <vulkan/vulkan.hpp>

vk::Image createImage(
    vk::Device device,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage)
{
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    vk::Image image = device.createImage(imageInfo, nullptr);
    return image;
}

vk::DeviceMemory allocateAndBindMemory(
    Device& device, vk::Image image, vk::MemoryPropertyFlags memoryProperties)
{
    vk::MemoryRequirements memRequirements =
        static_cast<vk::Device>(device).getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        device.findMemoryType(memRequirements.memoryTypeBits, memoryProperties));

    vk::DeviceMemory memory = static_cast<vk::Device>(device).allocateMemory(allocInfo, nullptr);
    static_cast<vk::Device>(device).bindImageMemory(image, memory, 0);
    return memory;
}

vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView = device.createImageView(viewInfo, nullptr);
    return imageView;
}

Image::Image(
    Device& device,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memoryProperties)
    : mDevice(device),
      mExtent(extent),
      mFormat(format),
      mImage(createImage(mDevice, mExtent, mFormat, tiling, usage)),
      mMemory(allocateAndBindMemory(mDevice, mImage, memoryProperties)),
      mView(createImageView(mDevice, mImage, mFormat))
{
}

Image::~Image()
{
    //    static_cast<vk::Device>(mDevice).freeMemory(mMemory);
    //    static_cast<vk::Device>(mDevice).destroyImage(mImage);
}
