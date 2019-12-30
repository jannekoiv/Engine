
#include "../Include/Base.h"
#include "../Include/FramebufferSet.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Device;
class Model;
class FramebufferSet;
class SwapChain;
class Texture;

class Renderer {
public:
    Renderer(Device& device, SwapChain& swapChain, Texture& depthImage, std::vector<Model>& models);

    void drawFrame();

private:
    Device& mDevice;
    SwapChain& mSwapChain;
    Texture& mDepthImage;
    FramebufferSet mClearFramebufferSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
