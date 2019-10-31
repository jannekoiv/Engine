
#include "../Include/Semaphore.h"
#include "../Include/Device.h"
#include "../Include/Base.h"
#include <vulkan/vulkan.hpp>
#include <iostream>

Semaphore::Semaphore(Device& device) : mDevice(device)
{
    mSemaphore = static_cast<vk::Device>(mDevice).createSemaphore({});
    vk::SemaphoreCreateInfo i;
}

Semaphore::~Semaphore()
{
}
