#pragma once

//#define VULKAN_HPP_NO_SMART_HANDLE

#include <vulkan/vulkan.hpp>
//#define GLM_FORCE_ALIGNED
//#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

#define UNUSED(x) (void)(x)

int32_t read_int(std::ifstream& file);

uint32_t read_uint(std::ifstream& file);

size_t read_size(std::ifstream& file);

float read_float(std::ifstream& file);

std::string read_string(std::ifstream& file);

std::vector<char> read_file(const std::string& filename);

bool has_stencil_component(vk::Format format);

//vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool);

#define single_time_command(OBJ, FN, ...)                                                            \
    [](auto&& obj, auto&&... args) -> decltype(auto) {                                             \
        vk::CommandBuffer command_buffer = obj.beginSingleTimeCommand();                            \
        command_buffer.FN(args...);                                                                 \
        obj.end_single_time_command(command_buffer);                                                   \
    }(OBJ, __VA_ARGS__);

//void endSingleTimeCommands(
//    vk::Device device,
//    vk::CommandPool commandPool,
//    vk::CommandBuffer commandBuffer,
//    vk::Queue queue);
