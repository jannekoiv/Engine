
#pragma once

#include "../Include/Base.h"

class Engine;

class DescriptorPool {
public:
    DescriptorPool(Engine& engine);

    ~DescriptorPool();

    operator vk::DescriptorPool() const;

private:
    Engine& mEngine;
    vk::DescriptorPool mDescriptorPool;
};
