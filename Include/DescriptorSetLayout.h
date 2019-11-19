
#pragma once

#include "../Include/Base.h"

class Device;

class DescriptorSetLayout {
public:
    DescriptorSetLayout(Device& device);
    ~DescriptorSetLayout();

    operator vk::DescriptorSetLayout() const
    {
        return mLayout;
    }

private:
    Device& mDevice;
    vk::DescriptorSetLayout mLayout;
};
