#pragma once

#include "base.h"

class Device;

struct DescriptorWrite {
    int binding;
    int array_element;
    int descriptor_count;
    void* infos;
};

class DescriptorSet {
public:
    DescriptorSet(const DescriptorSet&) = delete;

    DescriptorSet(DescriptorSet&& rhs);

    DescriptorSet(
        vk::Device device,
        std::vector<vk::DescriptorSetLayoutBinding> bindings,
        vk::DescriptorSet descriptor_set,
        vk::DescriptorSetLayout layout);

    DescriptorSet& operator=(const DescriptorSet&) = delete;

    DescriptorSet& operator=(DescriptorSet&&) = delete;

    operator vk::DescriptorSet()
    {
        return _descriptor_set;
    }

    vk::DescriptorSetLayout layout()
    {
        return _layout;
    }

    void write_descriptors(std::vector<DescriptorWrite> descriptor_writes);

private:
    vk::Device _device;
    std::vector<vk::DescriptorSetLayoutBinding> _bindings;
    vk::DescriptorSet _descriptor_set;
    vk::DescriptorSetLayout _layout;
};
