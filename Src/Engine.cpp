#include "../Include/Engine.h"
#include <fstream>

GLFWwindow* initWindow(const int width, const int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window =
        glfwCreateWindow(width, height, "Team Leprabakteeri Rendering Engine", nullptr, nullptr);
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
          1,
          vk::Extent3D{mSwapChain.extent()},
          findDepthAttachmentFormat(mDevice),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge},
      mDescriptorManager{mDevice},
      mRenderer{mDevice, mSwapChain, mDepthTexture},
      mSkybox{mDevice, mDescriptorManager, mSwapChain, mDepthTexture}
{
}

Model Engine::createModelFromFile(std::string filename)
{
    return ::createModelFromFile(mDevice, mDescriptorManager, mSwapChain, mDepthTexture, filename);
}

void Engine::initRenderer(std::vector<Model>& models)
{
    mRenderer.createCommandBuffers(models, mSkybox);
}

void Engine::drawFrame()
{
    mRenderer.drawFrame();
}



