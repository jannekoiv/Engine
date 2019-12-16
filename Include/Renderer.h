
#include "../Include/Base.h"
#include "../Include/RenderPass.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class Model;
class Framebuffer;
class SwapChain;
class Image;

class Renderer {
public:
    Renderer(Device& device, SwapChain& swapChain, Image& depthImage, std::vector<Model>& models);

    void drawFrame();

private:
    Device& mDevice;
    SwapChain& mSwapChain;
    Image& mDepthImage;
    RenderPass mClearRenderPass;
    std::vector<Framebuffer> mClearFramebuffers;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
