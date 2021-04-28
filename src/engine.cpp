#include "engine.h"

GLFWwindow* init_window(const int width, const int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        width, height, "Totaalinen Yliruletus Rendering Engine", nullptr, nullptr);
//    glfwSetKeyCallback(window, keyCallback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}

int init_paskaa()
{
    std::cout << "PASKAA\n";
    return 0;
}

Engine::Engine(const int width, const int height, const bool enable_validation_layers)
    : _window{init_window(width, height)},
      _device{_window, enable_validation_layers},
      _swap_chain{_device},
      _depth_texture{
          _device,
          vk::ImageViewType::e2D,
          1,
          vk::Extent3D{_swap_chain.extent().width, _swap_chain.extent().height, 1},
          find_depth_attachment_optimal(_device),
          vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eDepthStencilAttachment |
              vk::ImageUsageFlagBits::eSampled,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
          vk::SamplerAddressMode::eClampToEdge}
      // _descriptor_manager{_device},
      // _texture_manager{_device},
      // _renderer{_device, _descriptor_manager, _swap_chain, _depth_texture}
      //mSkybox{mDevice, mDescriptorManager, mTextureManager, mSwapChain, mDepthTexture},
      // mLight{mDevice, mDescriptorManager, mTextureManager, mSwapChain}
      // _quad{_device, _descriptor_manager, _texture_manager, _swap_chain}
{
    std::cout << "Engine initialized\n";
}

Engine::~Engine()
{
}

// Object Engine::create_object_from_file(std::string filename)
// {
//     std::ifstream file(filename, std::ios::in | std::ios::binary);
//     if (!file.is_open()) {
//         throw std::runtime_error("Failed to open object file!");
//     }

//     std::string header = read_string(file);
//     if (header != "paskaformaatti 1.0") {
//         std::cout << "HEADER\n";
//         throw std::runtime_error("Header file not matching!");
//     }

//     glm::mat4 world_matrix;
//     file.read(reinterpret_cast<char*>(&world_matrix), sizeof(glm::mat4));

//     uint32_t vertex_count = read_int(file);
//     std::cout << "vertex count " << vertex_count << std::endl;
//     std::vector<Vertex> vertices(vertex_count);
//     for (Vertex& vertex : vertices) {
//         vertex.position.x = read_float(file);
//         vertex.position.y = read_float(file);
//         vertex.position.z = read_float(file);
//         vertex.normal.x = read_float(file);
//         vertex.normal.y = read_float(file);
//         vertex.normal.z = read_float(file);
//         vertex.tex_coord.x = read_float(file);
//         vertex.tex_coord.y = read_float(file);
//     }

//     uint32_t index_count = read_int(file);
//     std::cout << "index count " << index_count << std::endl;
//     std::vector<uint32_t> indices(index_count);
//     for (uint32_t& index : indices) {
//         index = read_int(file);
//     }

//     auto material_filename = read_string(file);
//     using Json = nlohmann::json;
//     std::ifstream material_file{material_filename};
//     if (!material_file.is_open()) {
//         throw std::runtime_error("Failed to open material file!");
//     }
//     Json json;
//     material_file >> json;

//     //Material material =
//     //    createMaterialFromFile(device, descriptorManager, textureManager, swapChain, &depthTexture, materialFilename);

//     auto keyframe_count = read_int(file);
//     std::cout << "keyframeCount " << keyframe_count << "\n";
//     std::vector<glm::mat4> keyframes(keyframe_count);
//     for (auto i = 0; i < keyframe_count; ++i) {
//         file.read(reinterpret_cast<char*>(&keyframes[i]), sizeof(glm::mat4));
//         keyframes[i][3][0] *= 0.001f;
//         keyframes[i][3][1] *= 0.001f;
//         keyframes[i][3][2] *= 0.001f;
//     }

//     file.close();
//     return Object{
//         _device,
//         _descriptor_manager,
//         _texture_manager,
//         _renderer.descriptor_set(),
//         _swap_chain,
//         _depth_texture,
//         world_matrix,
//         vertices,
//         indices,
//         json,
//         nullptr,
//         keyframes};
// }

// void Engine::draw_frame(std::vector<Object>& objects)
// {
//     _camera.update();
    // const glm::mat4& world = mLight.worldMatrix();
    // mRenderer.updateUniformBuffer(
    //     mCamera.viewMatrix(),
    //     mCamera.projMatrix(),
    //     mLight.projMatrix() * mLight.viewMatrix(),
    //     {world[2][0], world[2][1], world[2][2]});

    // for (Object& object : objects) {
    //     object.updateUniformBuffer(
    //         mCamera.viewMatrix(),
    //         mCamera.projMatrix(),
    //         mLight.projMatrix() * mLight.viewMatrix(),
    //         {world[2][0], world[2][1], world[2][2]});
    // }

    //mSkybox.updateUniformBuffer(glm::mat4(glm::mat3(mCamera.viewMatrix())), mCamera.projMatrix());
    // _quad.update_uniform_buffer();

    //mLight.drawFrame(objects, mSwapChain.extent());
    // _renderer.draw_frame(objects);
// }
