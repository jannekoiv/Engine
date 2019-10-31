
#pragma once

#include "../Include/Base.h"

class Device;

class Sampler {
public:
    Sampler(Device& device, vk::SamplerAddressMode addressMode);

    ~Sampler();

    operator vk::Sampler() const
    {
        return mSampler;
    }

private:
    Device& mDevice;
    vk::Sampler mSampler;
};
