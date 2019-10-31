#include "../Include/Sampler.h"
#include "../Include/Device.h"
#include "../Include/Base.h"
#include <vulkan/vulkan.hpp>

Sampler::Sampler(Device& device, vk::SamplerAddressMode addressMode)
    : mDevice(device)
{
    vk::SamplerCreateInfo samplerInfo;

    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;

    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;

    samplerInfo.anisotropyEnable = true;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = false;

    samplerInfo.compareEnable = false;
    samplerInfo.compareOp = vk::CompareOp::eAlways;

    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    mSampler = static_cast<vk::Device>(mDevice).createSampler(samplerInfo, nullptr);
}

Sampler::~Sampler()
{
}
