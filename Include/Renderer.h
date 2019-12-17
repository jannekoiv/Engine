
#include "../Include/Base.h"
#include "../Include/Framebuffer.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class Model;
class FramebufferSet;
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
    FramebufferSet mClearFramebufferSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
