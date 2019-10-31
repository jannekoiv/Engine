#pragma once

#include "../Include/Base.h"

class Device;
class Image;

class ImageView {
public:
    ImageView(Image& image, vk::ImageAspectFlags aspectFlags);

    ImageView(Device& device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

    ~ImageView();

    operator vk::ImageView() const
    {
        return mImageView;
    }

    vk::Format format() const
    {
        return mFormat;
    }

private:
    vk::ImageView mImageView;
    vk::Format mFormat;
};
