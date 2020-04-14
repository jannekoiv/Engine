
#pragma once

#include "../Include/Base.h"
#include "../Include/DescriptorSet.h"
#include <list>

const int maxSets = 10;

class Device;

class DescriptorContainer {
public:
    DescriptorContainer(const DescriptorContainer&) = delete;

    DescriptorContainer(DescriptorContainer&&) = delete;

    DescriptorContainer(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings);

    ~DescriptorContainer();

    DescriptorContainer& operator=(const DescriptorContainer&) = delete;

    DescriptorContainer& operator=(DescriptorContainer&&) = delete;

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
    DescriptorManager(const DescriptorManager&) = delete;

    DescriptorManager(DescriptorManager&&) = delete;

    DescriptorManager(Device& device);

    DescriptorManager& operator=(const DescriptorManager&) = delete;

    DescriptorManager& operator=(DescriptorManager&&) = delete;

    DescriptorSet createDescriptorSet(std::vector<vk::DescriptorSetLayoutBinding> bindings);

private:
    Device& mDevice;
    std::list<DescriptorContainer> mContainers;
};
