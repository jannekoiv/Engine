#pragma once

#include "ext_includes.h"

class ShaderCompiler {
public:
    ShaderCompiler(vk::Device device);
    ~ShaderCompiler();

    vk::ShaderModule compile(
        const vk::ShaderStageFlagBits shader_type, const char* pshader);
private:
    vk::Device _device;
};
