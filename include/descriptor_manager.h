
#pragma once

#include "base.h"
#include "descriptor_set.h"
#include <list>

const int max_sets = 10;

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
        return _bindings;
    }

    int sets_left()
    {
        return _sets_left;
    }

    vk::DescriptorSetLayout layout()
    {
        return _layout;
    }

    vk::DescriptorSet create_descriptor_set();

private:
    vk::Device _device;
    std::vector<vk::DescriptorSetLayoutBinding> _bindings;
    int _sets_left;
    vk::DescriptorSetLayout _layout;
    std::list<vk::DescriptorPool> _pools;
};

class DescriptorManager {
public:
    DescriptorManager(const DescriptorManager&) = delete;

    DescriptorManager(DescriptorManager&&) = delete;

    DescriptorManager(Device& device);

    DescriptorManager& operator=(const DescriptorManager&) = delete;

    DescriptorManager& operator=(DescriptorManager&&) = delete;

    DescriptorSet create_descriptor_set(std::vector<vk::DescriptorSetLayoutBinding> bindings);

private:
    Device& _device;
    std::list<DescriptorContainer> _containers;
};
