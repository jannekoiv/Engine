
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/Engine.h"
#include "../Include/Image.h"
#include "../Include/Model.h"
#include "../Include/Sampler.h"

struct UboWorldView {
    glm::mat4* worldView = nullptr;
};

class Renderer {
public:
    Renderer(Engine& engine, std::vector<Model*> models);
    void drawFrame();

private:
    Engine& mEngine;
    vk::DescriptorSet mDescriptorSet;
    std::vector<vk::CommandBuffer> mCommandBuffers;
    UboWorldView mUboWorldView;
    Buffer mUniformBuffer;
    vk::Semaphore mImageAvailableSemaphore;
    vk::Semaphore mRenderFinishedSemaphore;
};
