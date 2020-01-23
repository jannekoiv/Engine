#include "../Include/Engine.h"
#include <fstream>

GLFWwindow* initWindow(const int width, const int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(width, height, "Team Leprabakteeri Rendering Engine", nullptr, nullptr);
    //glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}

Engine::Engine(const int width, const int height, const bool enableValidationLayers)
    : mWindow{initWindow(width, height)},
      mDevice{mWindow, enableValidationLayers},
      mSwapChain{mDevice},
      mDepthTexture{
          mDevice,
          vk::ImageViewType::e2D,
          1,
          vk::Extent3D{mSwapChain.extent()},
          findDepthAttachmentFormat(mDevice),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge},
      mDescriptorManager{mDevice},
      mRenderer{mDevice, mSwapChain, mDepthTexture},
      mSkybox{mDevice, mDescriptorManager, mSwapChain, mDepthTexture},
      mLight{mDevice, mDescriptorManager, mSwapChain},
      mQuad{mDevice, mDescriptorManager, mSwapChain, mLight.mMaterial.texture()}
{
    std::cout << "Engine initialized\n";
}

Model Engine::createModelFromFile(std::string filename)
{
    return ::createModelFromFile(mDevice, mDescriptorManager, mSwapChain, &mDepthTexture, filename);
}

void Engine::initRenderer(std::vector<Model>& models)
{
    mRenderer.createCommandBuffers(models, mSkybox, mQuad, &mLight);
    mLight.createCommandBuffers(models, mSwapChain.extent());
}

void Engine::drawFrame()
{
    mLight.drawFrame();
    mRenderer.drawFrame();
}
