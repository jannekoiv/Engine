#include "../Include/DescriptorPool.h"
#include "../Include/Device.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.hpp>

DescriptorPool::DescriptorPool(Device& device) : mDevice(device)
{
    std::array<vk::DescriptorPoolSize, 2> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = 10;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = 10;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.maxSets = 10;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    mDescriptorPool = static_cast<vk::Device>(mDevice).createDescriptorPool(poolInfo, nullptr);
}

DescriptorPool::~DescriptorPool()
{
    //static_cast<vk::Device>(mDevice).destroyDescriptorPool(mDescriptorPool);
}

DescriptorPool::operator vk::DescriptorPool() const
{
    return mDescriptorPool;
}
