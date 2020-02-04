#include "../Include/Engine.h"
#include <fstream>

GLFWwindow* initWindow(const int width, const int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(width, height, "Totaalinen Yliruletus Rendering Engine", nullptr, nullptr);
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
          vk::Extent3D{mSwapChain.extent().width, mSwapChain.extent().height, 1},
          findDepthAttachmentFormat(mDevice),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge},
      mDescriptorManager{mDevice},
      mRenderer{mDevice, mSwapChain, mDepthTexture},
      mSkybox{mDevice, mDescriptorManager, mSwapChain, mDepthTexture},
      mLight{mDevice, mDescriptorManager, mSwapChain},
      mQuad{mDevice, mDescriptorManager, mSwapChain, mLight.depthTexture()}
{
    std::cout << "Engine initialized\n";
}

Model Engine::createModelFromFile(std::string filename)
{
    return ::createModelFromFile(
        mDevice, mDescriptorManager, mSwapChain, mDepthTexture, &mLight.depthTexture(), filename);
}

void Engine::drawFrame(std::vector<Model>& models)
{
    for (Model& model : models) {
        model.uniform().view = mCamera.viewMatrix();
        model.uniform().proj = mCamera.projMatrix();

        model.uniform().lightSpace = mLight.projMatrix() * mLight.viewMatrix();
        glm::mat4 world = mLight.worldMatrix();
        model.uniform().lightDir = {world[2][0], world[2][1], world[2][2]};

        model.updateUniformBuffer();
    }

    mSkybox.uniform().worldView = mCamera.viewMatrix();
    mSkybox.uniform().proj = mCamera.projMatrix();
    mSkybox.updateUniformBuffer();

    mQuad.updateUniformBuffer();

    mLight.drawFrame(models, mSwapChain.extent());
    mRenderer.drawFrame(models, mSkybox, mQuad, mLight);
}
