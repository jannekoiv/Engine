//#pragma once
//
//#include "../Include/Base.h"
//
//class Device;
//
//class CommandPool {
//public:
//    CommandPool(Device& device);
//
//    ~CommandPool();
//
//    operator vk::CommandPool() const
//    {
//        return mCommandPool;
//    }
//
//    std::vector<vk::CommandBuffer> allocateCommandBuffers(
//        vk::CommandBufferLevel level, size_t count);
//
//    vk::CommandBuffer beginSingleTimeCommands();
//
//    void endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::Queue queue);
//
//private:
//    Device& mDevice;
//    vk::CommandPool mCommandPool;
//};
//
