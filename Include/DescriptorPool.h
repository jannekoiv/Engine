
#pragma once

#include "../Include/Base.h"

class Device;

class DescriptorPool {
public:
    DescriptorPool(Device& engine);

    ~DescriptorPool();

    operator vk::DescriptorPool() const;

private:
    Device& mDevice;
    vk::DescriptorPool mDescriptorPool;
};
