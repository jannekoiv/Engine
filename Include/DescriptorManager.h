
#pragma once

#include "../Include/Base.h"
#include "../Include/DescriptorSet.h"
#include <list>

class Device;

const int maxSets = 10;

class DescriptorContainer {
public:
    DescriptorContainer(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings);

    std::vector<vk::DescriptorSetLayoutBinding> bindings()
    {
        return mBindings;
    }

    int setsLeft()
    {
        return mSetsLeft;
    }

    vk::DescriptorSetLayout layout()
    {
        return mLayout;
    }

    vk::DescriptorSet createDescriptorSet();

private:
    vk::Device mDevice;
    std::vector<vk::DescriptorSetLayoutBinding> mBindings;
    int mSetsLeft;
    vk::DescriptorSetLayout mLayout;
    std::list<vk::DescriptorPool> mPools;
};

class DescriptorManager {
public:
    DescriptorManager(Device& device);

    ~DescriptorManager();

    DescriptorSet createDescriptorSet(std::vector<vk::DescriptorSetLayoutBinding> bindings);

private:
    Device& mDevice;
    std::list<DescriptorContainer> mContainers;
};
