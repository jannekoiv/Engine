
#include "descriptor_manager.h"
#include "device.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.hpp>

DescriptorSet::DescriptorSet(DescriptorSet&& rhs)
    : _device(rhs._device), _bindings{rhs._bindings}, _descriptor_set{rhs._descriptor_set}, _layout{rhs._layout}
{
    rhs._descriptor_set = nullptr;
    rhs._layout = nullptr;
}

DescriptorSet::DescriptorSet(
    vk::Device device,
    std::vector<vk::DescriptorSetLayoutBinding> bindings,
    vk::DescriptorSet descriptor_set,
    vk::DescriptorSetLayout layout)
    : _device(device), _bindings{bindings}, _descriptor_set{descriptor_set}, _layout{layout}
{
}

void DescriptorSet::write_descriptors(std::vector<DescriptorWrite> descriptor_writes)
{
    std::vector<vk::WriteDescriptorSet> writes(descriptor_writes.size());

    for (int i = 0; i < descriptor_writes.size(); i++) {
        writes[i].dstSet = _descriptor_set;
        writes[i].dstBinding = descriptor_writes[i].binding;
        writes[i].dstArrayElement = descriptor_writes[i].array_element;
        writes[i].descriptorCount = descriptor_writes[i].descriptor_count;
        writes[i].descriptorType = _bindings[descriptor_writes[i].binding].descriptorType;

        if (_bindings[descriptor_writes[i].binding].descriptorType == vk::DescriptorType::eUniformBuffer) {
            writes[i].pBufferInfo = static_cast<vk::DescriptorBufferInfo*>(descriptor_writes[i].infos);
            writes[i].pImageInfo = nullptr;
        } else {
            writes[i].pBufferInfo = nullptr;
            writes[i].pImageInfo = static_cast<vk::DescriptorImageInfo*>(descriptor_writes[i].infos);
        }

        writes[i].pTexelBufferView = nullptr;
    }

    _device.updateDescriptorSets(writes, nullptr);
}
