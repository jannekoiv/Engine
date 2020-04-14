#pragma once
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/Camera.h"
#include "../Include/DescriptorManager.h"
#include "../Include/Device.h"
#include "../Include/DirectionalLight.h"
#include "../Include/Model.h"
#include "../Include/Quad.h"
#include "../Include/Renderer.h"
#include "../Include/Skybox.h"
#include "../Include/SwapChain.h"
#include "../Include/Texture.h"
#include "../Include/TextureManager.h"

class Engine {
public:
    Engine(const Engine&) = delete;

    Engine(Engine&&) = delete;

    Engine(const int width, const int height, const bool enableValidationLayers);

    ~Engine();

    Engine& operator=(const Engine&) = delete;

    Engine& operator=(Engine&&) = delete;

    void setKeyCallback(GLFWkeyfun callback)
    {
        glfwSetKeyCallback(mWindow, callback);
    }

    Model createModelFromFile(std::string filename);

    GLFWwindow* window()
    {
        return mWindow;
    }

    void drawFrame(std::vector<Model>& models);

    Camera& camera()
    {
        return mCamera;
    }

    Device& device()
    {
        return mDevice;
    }

    TextureManager& textureManager()
    {
        return mTextureManager;
    }

    //
    //    SwapChain& swapChain()
    //    {
    //        return mSwapChain;
    //    }
    //
    //    RenderPass& renderPass()
    //    {
    //        return mRenderPass;
    //    }
    //
    //    vk::Framebuffer frameBuffer(int index) const
    //    {
    //        return mFrameBuffers[index];
    //    }
    //
    //    size_t frameBufferCount() const
    //    {
    //        return mFrameBuffers.size();
    //    }
    //
    //    std::vector<vk::Framebuffer>& frameBuffers()
    //    {
    //        return mFrameBuffers;
    //    }
    //
private:
    GLFWwindow* mWindow;
    Device mDevice;
    SwapChain mSwapChain;
    Texture mDepthTexture;
    DescriptorManager mDescriptorManager;
    TextureManager mTextureManager;
    Renderer mRenderer;

public:
    Camera mCamera;
    Skybox mSkybox;
    DirectionalLight mLight;
    Quad mQuad;
};
