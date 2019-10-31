

#pragma once

#include "../Include/Base.h"

class Device;

class ShaderModule {
public:
    ShaderModule(Device& device, std::string filename);

    ~ShaderModule();

    operator vk::ShaderModule() const
    {
        return mShaderModule;
    }

private:
    Device& mDevice;
    vk::ShaderModule mShaderModule;
};
