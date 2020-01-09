
#include "../Include/Base.h"
#include <fstream>

int32_t readInt(std::ifstream& file)
{
    int32_t v = 0;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

uint32_t readUInt(std::ifstream& file)
{
    uint32_t v = 0;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

size_t readSize(std::ifstream& file)
{
    size_t v = 0;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

float readFloat(std::ifstream& file)
{
    float v = 0;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

std::string readString(std::ifstream& file)
{
    int32_t len = readInt(file);
    char tmp[100];
    memset(tmp, 0, sizeof(tmp));
    file.read(tmp, len);
    std::vector<char> vec;
    return std::string(tmp);
}

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
