
#include "../Include/ShaderModule.h"
#include "../Include/Device.h"
#include "../Include/Base.h"
#include <vulkan/vulkan.hpp>

ShaderModule::ShaderModule(Device& device, std::string filename) : mDevice(device)
{
    auto code = readFile(filename);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    mShaderModule = static_cast<vk::Device>(mDevice).createShaderModule(createInfo, nullptr);
}

ShaderModule::~ShaderModule()
{
    static_cast<vk::Device>(mDevice).destroyShaderModule(mShaderModule);
}
