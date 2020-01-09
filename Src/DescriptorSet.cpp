
#include "../Include/DescriptorManager.h"
#include "../Include/Device.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.hpp>

void DescriptorSet::writeDescriptors(std::vector<DescriptorWrite> descriptorWrites)
{
    std::vector<vk::WriteDescriptorSet> writes(descriptorWrites.size());

    for (int i = 0; i < descriptorWrites.size(); i++) {
        writes[i].dstSet = mDescriptorSet;
        writes[i].dstBinding = descriptorWrites[i].binding;
        writes[i].dstArrayElement = descriptorWrites[i].arrayElement;
        writes[i].descriptorCount = descriptorWrites[i].descriptorCount;
        writes[i].descriptorType = mBindings[descriptorWrites[i].binding].descriptorType;

        if (mBindings[descriptorWrites[i].binding].descriptorType ==
            vk::DescriptorType::eUniformBuffer) {
            writes[i].pBufferInfo =
                static_cast<vk::DescriptorBufferInfo*>(descriptorWrites[i].infos);
            writes[i].pImageInfo = nullptr;
        } else {
            writes[i].pBufferInfo = nullptr;
            writes[i].pImageInfo = static_cast<vk::DescriptorImageInfo*>(descriptorWrites[i].infos);
        }

        writes[i].pTexelBufferView = nullptr;
    }

    mDevice.updateDescriptorSets(writes, nullptr);
}
