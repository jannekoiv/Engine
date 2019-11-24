#include "../Include/DescriptorManager.h"
#include "../Include/Device.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.hpp>

vk::DescriptorSetLayout createDescriptorSetLayout(
    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(layoutInfo, nullptr);
    return layout;
}

vk::DescriptorPool createDescriptorPool(
    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    std::vector<vk::DescriptorPoolSize> poolSizes(bindings.size());

    for (int i = 0; i < poolSizes.size(); i++) {
        poolSizes[i].type = bindings[i].descriptorType;
        poolSizes[i].descriptorCount = bindings[i].descriptorCount * maxSets;
    }

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    vk::DescriptorPool pool = device.createDescriptorPool(poolInfo, nullptr);
    return pool;
}

//std::list<vk::DescriptorPool> createPoolList(
//    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
//{
//
//}

DescriptorContainer::DescriptorContainer(
    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
    : mDevice{device},
      mBindings{bindings},
      mSetsLeft(maxSets),
      mLayout{createDescriptorSetLayout(mDevice, bindings)},
      mPools{createDescriptorPool(mDevice, bindings)}
{
}

vk::DescriptorSet DescriptorContainer::createDescriptorSet()
{
    if (mSetsLeft == 0) {
        std::cout << "POOL IS FULL\n";
        mPools.push_back(createDescriptorPool(mDevice, mBindings));
        mSetsLeft = maxSets;
    }

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = mPools.back();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &mLayout;

    vk::DescriptorSet descriptorSet = mDevice.allocateDescriptorSets(allocInfo).front();
    mSetsLeft--;
    return descriptorSet;
}

DescriptorManager::DescriptorManager(Device& device) : mDevice(device)
{
}

DescriptorManager::~DescriptorManager()
{
    //static_cast<vk::Device>(mDevice).destroyDescriptorPool(mDescriptorPool);
}

DescriptorContainer* findDescriptorContainer(
    std::list<DescriptorContainer>& containers,
    std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    for (std::list<DescriptorContainer>::iterator it = containers.begin(); it != containers.end();
         it++) {
        if (it->bindings() == bindings) {
            return &(*it);
        }
    }
    return nullptr;
}

DescriptorSet DescriptorManager::createDescriptorSet(
    std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    auto container = findDescriptorContainer(mContainers, bindings);

    if (!container) {
        std::cout << "Container not found\n";
        container = &mContainers.emplace_back(mDevice, bindings);
    }

    return DescriptorSet{
        mDevice, container->bindings(), container->createDescriptorSet(), container->layout()};
}
