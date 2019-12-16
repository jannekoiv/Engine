#pragma once

//#define VULKAN_HPP_NO_SMART_HANDLE

#include <Windows.h>
#include <vulkan/vulkan.hpp>
//#define GLM_FORCE_ALIGNED
//#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)(x)

std::vector<char> readFile(const std::string& filename);

bool hasStencilComponent(vk::Format format);

//vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool);

#define singleTimeCommand(OBJ, FN, ...)                                                            \
    [](auto&& obj, auto&&... args) -> decltype(auto) {                                             \
        vk::CommandBuffer commandBuffer = obj.beginSingleTimeCommand();                            \
        commandBuffer.FN(args...);                                                                 \
        obj.endSingleTimeCommand(commandBuffer);                                                   \
    }(OBJ, __VA_ARGS__);

//void endSingleTimeCommands(
//    vk::Device device,
//    vk::CommandPool commandPool,
//    vk::CommandBuffer commandBuffer,
//    vk::Queue queue);
