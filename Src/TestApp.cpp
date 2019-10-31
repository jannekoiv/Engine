#include <iostream>
#include "GLFW/glfw3.h"
#include "../Include/Engine.h"
#include "../Include/Renderer.h"
#include "../Include/Model.h"
#include <bitset>
#include <functional>
#include <fstream>

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

void mainLoop(GLFWwindow* window, Renderer& renderer, std::vector<Model*> models)
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
        renderer.drawFrame();
    }
}

int main()
{
    InitInfo initInfo;
    GLFWwindow* window = initWindow(initInfo.width, initInfo.height);

    Engine engine(initInfo, window);

    std::vector<Model*> models;

    for (int i = 0; i < 4; i++) {
        Model* model = new Model(engine, "/home/jak/apina.dat");
        models.push_back(model);
    }

    Renderer renderer(engine, models);

    mainLoop(window, renderer, models);

    for (Model* model : models) {
        delete model;
    }

    return 0;
}
