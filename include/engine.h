#pragma once
#include "base.h"
#include "buffer.h"
#include "camera.h"
#include "descriptor_manager.h"
#include "device.h"
#include "directional_light.h"
#include "object.h"
#include "quad.h"
#include "renderer.h"
#include "skybox.h"
#include "swap_chain.h"
#include "texture.h"
#include "texture_manager.h"

class Engine {
public:
    Engine(const Engine&) = delete;

    Engine(Engine&&) = delete;

    Engine(const int width, const int height, const bool enable_validation_layers);

    ~Engine();

    Engine& operator=(const Engine&) = delete;

    Engine& operator=(Engine&&) = delete;

    void set_key_callback(GLFWkeyfun callback)
    {
        glfwSetKeyCallback(_window, callback);
    }

    Object create_object_from_file(std::string filename);

    GLFWwindow* window()
    {
        return _window;
    }

    void draw_frame(std::vector<Object>& objects);

    Camera& camera()
    {
        return _camera;
    }

    Device& device()
    {
        return _device;
    }

    TextureManager& texture_manager()
    {
        return _texture_manager;
    }

private:
    GLFWwindow* _window;
    Device _device;
    SwapChain _swap_chain;
    Texture _depth_texture;
    DescriptorManager _descriptor_manager;
    TextureManager _texture_manager;
    Renderer _renderer;

public:
    Camera _camera;
    //Skybox mSkybox;
    // DirectionalLight mLight;
    Quad _quad;
};
