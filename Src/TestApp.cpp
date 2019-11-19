#include "../Include/Engine.h"
#include "../Include/Model.h"
#include "../Include/Renderer.h"
#include "GLFW/glfw3.h"
#include <bitset>
#include <fstream>
#include <functional>
#include <iostream>

#include <set>

#include <optional>

bool running = true;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(action);
    UNUSED(mods);
    if (key == GLFW_KEY_ESCAPE) {
        running = false;
    }
}

GLFWwindow* initWindow(const int width, const int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window =
        glfwCreateWindow(width, height, "Team Leprabakteeri Rendering Engine", nullptr, nullptr);
    glfwSetKeyCallback(window, keyCallback);
    return window;
}

float rot = 0.0f;

void mainLoop(
    GLFWwindow* window, Renderer& renderer, std::vector<Model*> models, SwapChain& swapChain)
{
    glm::vec3 positions[] = {
        {-2.0f, -2.0f, 0.0f}, {-2.0f, 2.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, {2.0f, -2.0f, 0.0f}};

    while (!glfwWindowShouldClose(window) && running) {
        glfwPollEvents();

        for (int i = 0; i < 4; i++) {
            Model* model = models[i];
            auto& worldView = model->mUniformBufferObject.worldView;
            auto& proj = model->mUniformBufferObject.proj;

            worldView = glm::lookAt(
                glm::vec3(0.0f, 0.0f, 10.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));
            worldView = glm::translate(worldView, positions[i]);
            worldView =
                glm::rotate(worldView, rot * static_cast<float>(i), glm::vec3(0.0f, 1.0f, 0.0f));

            proj = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 1000.0f);
            proj[1][1] *= -1.0f;

            model->updateUniformBuffer();
        }
        rot += 0.005f;
        renderer.drawFrame(swapChain);
    }
}

static std::vector<vk::Framebuffer> createFramebuffers(
    Device& device, SwapChain& swapChain, vk::ImageView depthImageView, RenderPass& renderPass)
{
    std::vector<vk::Framebuffer> frameBuffers(swapChain.imageViews().size());

    for (size_t i = 0; i < swapChain.imageViews().size(); i++) {
        std::vector<vk::ImageView> attachments = {swapChain.imageViews()[i], depthImageView};

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChain.extent().width;
        framebufferInfo.height = swapChain.extent().height;
        framebufferInfo.layers = 1;

        frameBuffers[i] =
            static_cast<vk::Device>(device).createFramebuffer(framebufferInfo, nullptr);
    }
    return frameBuffers;
}

int main()
{
    InitInfo initInfo{};
    GLFWwindow* window = initWindow(initInfo.width, initInfo.height);

    Device device{window, true, initInfo.validationLayers, initInfo.deviceExtensions};

    SwapChain swapChain{device, vk::Extent2D{initInfo.width, initInfo.height}};

    RenderPass renderPass{device, swapChain.format()};

    Image depthImage{
        device,
        vk::Extent3D(swapChain.extent()),
        renderPass.depthAttachmentFormat(),
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal};

    std::vector<vk::Framebuffer> frameBuffers =
        createFramebuffers(device, swapChain, depthImage.view(), renderPass);

    DescriptorSetLayout descriptorSetLayout{device};
    DescriptorPool descriptorPool{device};

    DescriptorManager descriptorManager{device};

    std::vector<Model*> models;

    for (int i = 0; i < 4; i++) {
        Model* model = new Model(
            device, descriptorManager, swapChain, renderPass, "d:/apina.dat");
        models.push_back(model);
    }

    std::cout << "Models loaded\n";

    Renderer renderer(device, models, frameBuffers, renderPass, swapChain);

    std::cout << "Renderer ok\n";

    mainLoop(window, renderer, models, swapChain);

    for (Model* model : models) {
        delete model;
    }

    return 0;
}
