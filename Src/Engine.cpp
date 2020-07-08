#include "../Include/Engine.h"
#include <fstream>

GLFWwindow* initWindow(const int width, const int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        width, height, "Totaalinen Yliruletus Rendering Engine", nullptr, nullptr);
    //glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}

int initPaskaa()
{
    std::cout << "PASKAA\n";
    return 0;
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
          vk::ImageUsageFlagBits::eDepthStencilAttachment |
              vk::ImageUsageFlagBits::eSampled,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge},
      mDescriptorManager{mDevice},
      mTextureManager{mDevice},
      mRenderer{mDevice, mDescriptorManager, mSwapChain, mDepthTexture},
      //mSkybox{mDevice, mDescriptorManager, mTextureManager, mSwapChain, mDepthTexture},
      mLight{mDevice, mDescriptorManager, mTextureManager, mSwapChain}
//mQuad{mDevice, mDescriptorManager, mTextureManager, mSwapChain, mLight.depthTexture()}
{
    std::cout << "Engine initialized\n";
}

Engine::~Engine()
{
}

Object Engine::createObjectFromFile(std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open object file!");
    }

    std::string header = readString(file);
    if (header != "paskaformaatti 1.0") {
        std::cout << "HEADER\n";
        throw std::runtime_error("Header file not matching!");
    }

    glm::mat4 worldMatrix;
    file.read(reinterpret_cast<char*>(&worldMatrix), sizeof(glm::mat4));

    uint32_t vertexCount = readInt(file);
    std::cout << "vertex count " << vertexCount << std::endl;
    std::vector<Vertex> vertices(vertexCount);
    for (Vertex& vertex : vertices) {
        vertex.position.x = readFloat(file);
        vertex.position.y = readFloat(file);
        vertex.position.z = readFloat(file);
        vertex.normal.x = readFloat(file);
        vertex.normal.y = readFloat(file);
        vertex.normal.z = readFloat(file);
        vertex.texCoord.x = readFloat(file);
        vertex.texCoord.y = readFloat(file);
    }

    uint32_t indexCount = readInt(file);
    std::cout << "index count " << indexCount << std::endl;
    std::vector<uint32_t> indices(indexCount);
    for (uint32_t& index : indices) {
        index = readInt(file);
    }

    auto materialFilename = readString(file);
    using Json = nlohmann::json;
    std::ifstream materialFile{materialFilename};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open material file!");
    }
    Json json;
    materialFile >> json;

    //Material material =
    //    createMaterialFromFile(device, descriptorManager, textureManager, swapChain, &depthTexture, materialFilename);

    auto keyframeCount = readInt(file);
    std::cout << "keyframeCount " << keyframeCount << "\n";
    std::vector<glm::mat4> keyframes(keyframeCount);
    for (auto i = 0; i < keyframeCount; ++i) {
        file.read(reinterpret_cast<char*>(&keyframes[i]), sizeof(glm::mat4));
        keyframes[i][3][0] *= 0.001f;
        keyframes[i][3][1] *= 0.001f;
        keyframes[i][3][2] *= 0.001f;
    }

    file.close();
    return Object{
        mDevice,
        mDescriptorManager,
        mTextureManager,
        mRenderer.descriptorSet(),
        mSwapChain,
        mDepthTexture,
        worldMatrix,
        vertices,
        indices,
        json,
        nullptr,
        keyframes};
}

void Engine::drawFrame(std::vector<Object>& objects)
{
    mCamera.update();
    const glm::mat4& world = mLight.worldMatrix();
    mRenderer.updateUniformBuffer(
        mCamera.viewMatrix(),
        mCamera.projMatrix(),
        mLight.projMatrix() * mLight.viewMatrix(),
        {world[2][0], world[2][1], world[2][2]});

    for (Object& object : objects) {
        object.updateUniformBuffer(
            mCamera.viewMatrix(),
            mCamera.projMatrix(),
            mLight.projMatrix() * mLight.viewMatrix(),
            {world[2][0], world[2][1], world[2][2]});
    }

    //mSkybox.updateUniformBuffer(glm::mat4(glm::mat3(mCamera.viewMatrix())), mCamera.projMatrix());
    //mQuad.updateUniformBuffer();

    //mLight.drawFrame(objects, mSwapChain.extent());
    mRenderer.drawFrame(objects);
}
