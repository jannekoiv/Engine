
#pragma once

#include "descriptor_set.h"
#include "framebuffer_set.h"
#include <nlohmann/json.hpp>

class DescriptorManager;
class TextureManager;
class Texture;

class Pipeline {
public:
    Pipeline(const Pipeline&) = delete;

    Pipeline(Pipeline&& rhs);

    Pipeline(
        Device& device,
        DescriptorManager& descriptor_manager,
        TextureManager& texture_manager,
        SwapChain& swap_chain,
        Texture* depth_texture,
        vk::VertexInputBindingDescription binding_description,
        std::vector<vk::VertexInputAttributeDescription> attribute_descriptions,
        vk::DescriptorSetLayout descriptor_set_layout1,
        vk::DescriptorSetLayout descriptor_set_layout2,
        const nlohmann::json& json);

    ~Pipeline();

    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline& operator=(Pipeline&&) = delete;

    operator vk::Pipeline() const
    {
        return _pipeline;
    }

    FramebufferSet& framebuffer_set()
    {
        return _framebuffer_set;
    }

    vk::PipelineLayout layout() const
    {
        return _pipeline_layout;
    }

    DescriptorSet& descriptor_set()
    {
        return _descriptor_set;
    }

private:
    Device& _device;
    FramebufferSet _framebuffer_set;
    Texture* _texture;
    DescriptorSet _descriptor_set;
    vk::PipelineLayout _pipeline_layout;
    vk::Pipeline _pipeline;
};
