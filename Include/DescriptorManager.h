
#pragma once

#include "../Include/Base.h"
#include <list>

class Device;

class DescriptorContainer {
public:
    DescriptorContainer(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings);

    std::vector<vk::DescriptorSetLayoutBinding> bindings()
    {
        return mBindings;
    }

    std::string name()
    {
        return mName;
    }

    vk::DescriptorSet createDescriptorSet();

//private:
    vk::Device mDevice;
    std::string mName;
    std::vector<vk::DescriptorSetLayoutBinding> mBindings;
    vk::DescriptorSetLayout mLayout;
    vk::DescriptorPool mPool;
};

class DescriptorManager {
public:
    DescriptorManager(Device& device);

    ~DescriptorManager();

    vk::DescriptorSetLayout lastLayout()
    {
        return mLastLayout;
    }

    vk::DescriptorSet createDescriptorSet(std::vector<vk::DescriptorSetLayoutBinding> bindings);

private:
    Device& mDevice;
    std::list<DescriptorContainer> mContainers;
    vk::DescriptorSetLayout mLastLayout;
};
