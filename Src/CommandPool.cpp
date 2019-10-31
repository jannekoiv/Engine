//#include "../Include/CommandPool.h"
//#include "../Include/Base.h"
//#include "../Include/Device.h"
//#include <iostream>
//#include <vector>
//#include <vulkan/vulkan.hpp>
//
//CommandPool::CommandPool(Device& device) : mDevice(device)
//{
//    std::cout << "QUEUE IS " << mDevice.queueFamilyIndices().graphics << "\n";
//    vk::CommandPoolCreateInfo commandPoolInfo({}, mDevice.queueFamilyIndices().graphics);
//    mCommandPool = static_cast<vk::Device>(mDevice).createCommandPool(commandPoolInfo);
//}
//
//CommandPool::~CommandPool()
//{
//    static_cast<vk::Device>(mDevice).destroyCommandPool(mCommandPool);
//}
//
//std::vector<vk::CommandBuffer> CommandPool::allocateCommandBuffers(
//    vk::CommandBufferLevel level, size_t count)
//{
//    vk::CommandBufferAllocateInfo allocInfo(mCommandPool, level, count);
//    return static_cast<vk::Device>(mDevice).allocateCommandBuffers(allocInfo);
//}
//
//vk::CommandBuffer CommandPool::beginSingleTimeCommands()
//{
//    std::cout << "Begin single time command\n";
//    vk::CommandBufferAllocateInfo allocInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, 1);
//    vk::CommandBuffer commandBuffer =
//        static_cast<vk::Device>(mDevice).allocateCommandBuffers(allocInfo).front();
//    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
//    commandBuffer.begin(beginInfo);
//    return commandBuffer;
//}
//
//void CommandPool::endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::Queue queue)
//{
//    std::cout << "End single time command\n\n";
//    commandBuffer.end();
//    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr);
//    queue.submit(submitInfo, nullptr);
//    queue.waitIdle();
//    static_cast<vk::Device>(mDevice).freeCommandBuffers(mCommandPool, commandBuffer);
//}
//
