#include "../Include/Engine.h"
#include "../Include/Material.h"
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
    GLFWwindow* window, Renderer& renderer, std::vector<Model>& models, SwapChain& swapChain)
{
    glm::vec3 positions[] = {
        {-2.0f, -2.0f, 0.0f}, {-2.0f, 2.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, {2.0f, -2.0f, 0.0f}};

    while (!glfwWindowShouldClose(window) && running) {
        glfwPollEvents();

        for (int i = 0; i < models.size(); i++) {
            Model& model = models[i];
            auto& worldView = model.uniform().worldView;
            auto& proj = model.uniform().proj;

            worldView = glm::lookAt(
                glm::vec3(-0.0f, 0.0f, 40.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));
            worldView = glm::translate(worldView, positions[i]);
            worldView =
                glm::rotate(worldView, rot * static_cast<float>(i), glm::vec3(0.0f, 1.0f, 0.0f));

            proj = glm::perspective(glm::radians(10.0f), 1920.0f / 1080.0f, 0.1f, 1000.0f);
            proj[1][1] *= -1.0f;

            model.updateUniformBuffer();
        }
        rot += 0.005f;
        renderer.drawFrame();
    }
}

class C {
public:
    C(int data)
    {
        mData = data;
    }

    C(const C&) = default;

    C(C&&) = delete;

    C& operator=(const C&) = default;

    C& operator=(C&&) = delete;

private:
    int mData;
};

int main()
{
    //std::vector<C> vec{};

    //vec.emplace_back(3);
    //vec.emplace_back(1);
    //vec.emplace_back(2);

    //std::sort(std::begin(vec), std::end(vec), less_than_key());

    //return 0;

    InitInfo initInfo{};
    GLFWwindow* window = initWindow(initInfo.width, initInfo.height);

    Device device{window, true, initInfo.validationLayers, initInfo.deviceExtensions};

    SwapChain swapChain{device, vk::Extent2D{initInfo.width, initInfo.height}};

    Texture depthImage{
        device,
        vk::Extent3D(swapChain.extent()),
        findDepthAttachmentFormat(device),
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eClampToEdge};

    DescriptorManager descriptorManager{device};

    std::vector<Model> models;

    for (int i = 0; i < 4; i++) {
        models.push_back(createModelFromFile(
            device, descriptorManager, swapChain, depthImage, "d:/meshes/Suzanne.dat"));
    }

    //std::sort(std::begin(models), std::end(models), less_than_key());

    std::cout << "Models loaded\n";

    Renderer renderer{device, swapChain, depthImage, models};

    std::cout << "Renderer ok\n";

    mainLoop(window, renderer, models, swapChain);

    return 0;
}
