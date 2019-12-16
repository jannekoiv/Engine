#pragma once

#include "../Include/Base.h"

class Device;

struct DescriptorWrite {
    int binding;
    int arrayElement;
    int descriptorCount;
    void* infos;
};

class DescriptorSet {
public:
    DescriptorSet(
        vk::Device device,
        std::vector<vk::DescriptorSetLayoutBinding> bindings,
        vk::DescriptorSet descriptorSet,
        vk::DescriptorSetLayout layout)
        : mDevice(device), mBindings{bindings}, mDescriptorSet{descriptorSet}, mLayout{layout}
    {
    }

    operator vk::DescriptorSet()
    {
        return mDescriptorSet;
    }

    vk::DescriptorSetLayout layout()
    {
        return mLayout;
    }

    void writeDescriptors(std::vector<DescriptorWrite> descriptorWrites);

private:
    vk::Device mDevice;
    std::vector<vk::DescriptorSetLayoutBinding> mBindings;
    vk::DescriptorSet mDescriptorSet;
    vk::DescriptorSetLayout mLayout;
};
