
#pragma once

#include "../Include/Base.h"

class Engine;

class DescriptorSetLayout {
public:
    DescriptorSetLayout(Engine& engine);
    ~DescriptorSetLayout();

    operator vk::DescriptorSetLayout() const
    {
        return mLayout;
    }

private:
    Engine& mEngine;
    vk::DescriptorSetLayout mLayout;
};
