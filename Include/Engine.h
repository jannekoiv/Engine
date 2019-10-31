#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/CommandPool.h"
#include "../Include/Device.h"
#include "../Include/Image.h"
#include "../Include/ImageView.h"
#include "../Include/RenderPass.h"
#include "../Include/Semaphore.h"
#include "../Include/SwapChain.h"

struct InitInfo {
    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    const bool enableValidationLayers = true;
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const int width = 1920;
    const int height = 1080;
};

struct Vertex {
    static vk::VertexInputBindingDescription getBindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Engine {
public:
    Engine(const InitInfo& initInfo, GLFWwindow* window);

    ~Engine();

    Device& device()
    {
        return mDevice;
    }

    SwapChain& swapChain()
    {
        return mSwapChain;
    }

    RenderPass& renderPass()
    {
        return mRenderPass;
    }

    vk::Framebuffer frameBuffer(int index) const
    {
        return mFrameBuffers[index];
    }

    size_t frameBufferCount() const
    {
        return mFrameBuffers.size();
    }

private:
    Device mDevice;
    SwapChain mSwapChain;
    RenderPass mRenderPass;
    Image mDepthImage;
    ImageView mDepthImageView;
    std::vector<vk::Framebuffer> mFrameBuffers;
};
