#include "../Include/Image.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include <vulkan/vulkan.hpp>

#include <iostream>

vk::Image createImage(
	vk::Device device,
	vk::Extent3D extent,
	vk::Format format,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage)
{
	extent.depth = 1;

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

bool isDepthFormat(vk::Format format) {
	std::vector<vk::Format> formats =
	{
		vk::Format::eD16Unorm,
		vk::Format::eX8D24UnormPack32,
		vk::Format::eD32Sfloat,
		vk::Format::eD16UnormS8Uint,
		vk::Format::eD24UnormS8Uint,
		vk::Format::eD32SfloatS8Uint
	};
	return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}

vk::ImageAspectFlagBits aspectMaskFromFormat(vk::Format format) {
	if (isDepthFormat(format)) {
		return vk::ImageAspectFlagBits::eDepth;
	}
	else {
		return vk::ImageAspectFlagBits::eColor;
	}
}

vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format)
{
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = image;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectMaskFromFormat(format);
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
