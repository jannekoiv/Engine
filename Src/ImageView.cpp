#include "../Include/ImageView.h"
#include "../Include/Device.h"
#include "../Include/Image.h"
#include <vulkan/vulkan.hpp>

ImageView::ImageView(Image& image, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = image.format();
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    mImageView = static_cast<vk::Device>(image.device()).createImageView(viewInfo, nullptr);
    mFormat = image.format();
}

ImageView::ImageView(
    Device& device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    mImageView = static_cast<vk::Device>(device).createImageView(viewInfo, nullptr);
    mFormat = format;
}

ImageView::~ImageView()
{
}
