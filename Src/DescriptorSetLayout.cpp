#include "../Include/DescriptorSetLayout.h"
#include "../Include/Device.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

DescriptorSetLayout::DescriptorSetLayout(Device& device) : mDevice(device)
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        uboLayoutBinding,
        samplerLayoutBinding,
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    mLayout = static_cast<vk::Device>(mDevice).createDescriptorSetLayout(layoutInfo, nullptr);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    //static_cast<vk::Device>(mDevice).destroyDescriptorSetLayout(mLayout);
}
