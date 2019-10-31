
#include "../Include/DescriptorSetLayout.h"
#include "../Include/Engine.h"
#include "../Include/Base.h"
#include <vulkan/vulkan.hpp>
#include <iostream>

DescriptorSetLayout::DescriptorSetLayout(Engine& engine) : mEngine(engine)
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 15;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.descriptorCount = 100;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        uboLayoutBinding,
        samplerLayoutBinding,
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    mLayout = static_cast<vk::Device>(mEngine.device()).createDescriptorSetLayout(layoutInfo, nullptr);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    static_cast<vk::Device>(mEngine.device()).destroyDescriptorSetLayout(mLayout);
}
