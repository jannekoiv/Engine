
#include "../Include/Base.h"
#include <fstream>

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

bool hasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

//vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool)
//{
//    vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
//    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo).front();
//    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
//    commandBuffer.begin(beginInfo);
//    return commandBuffer;
//}
//
//void endSingleTimeCommands(
//    vk::Device device,
//    vk::CommandPool commandPool,
//    vk::CommandBuffer commandBuffer,
//    vk::Queue queue)
//{
//    commandBuffer.end();
//    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr);
//    queue.submit(submitInfo, nullptr);
//    queue.waitIdle();
//    static_cast<vk::Device>(device).freeCommandBuffers(commandPool, commandBuffer);
//}
