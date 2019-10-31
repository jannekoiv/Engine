#include "../Include/DescriptorPool.h"
#include "../Include/Engine.h"
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>

DescriptorPool::DescriptorPool(Engine& engine) : mEngine(engine)
{
    std::array<vk::DescriptorPoolSize, 3> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = 300;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = 2000;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.maxSets = 3000000;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    mDescriptorPool = static_cast<vk::Device>(mEngine.device()).createDescriptorPool(poolInfo, nullptr);
}

DescriptorPool::~DescriptorPool()
{
    static_cast<vk::Device>(mEngine.device()).destroyDescriptorPool(mDescriptorPool);
}

DescriptorPool::operator vk::DescriptorPool() const
{
    return mDescriptorPool;
}
