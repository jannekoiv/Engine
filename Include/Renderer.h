
#include "../Include/Base.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class Model;
class RenderPass;
class SwapChain;

class Renderer {
public:
    Renderer(
        Device& device,
        std::vector<Model*> models,
        std::vector<vk::Framebuffer>& frameBuffers,
        RenderPass& renderPass,
        SwapChain& swapChain);
    void drawFrame(SwapChain& swapChain);

private:
    Device& mDevice;
    //    Engine& mEngine;
    vk::DescriptorSet mDescriptorSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    //    UboWorldView mUboWorldView;
    //    Buffer mUniformBuffer;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
