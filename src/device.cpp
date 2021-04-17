#include "device.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <vulkan/vulkan.hpp>

SwapChainSupportDetails::SwapChainSupportDetails(
    vk::SurfaceKHR surface, vk::PhysicalDevice device)
{
    capabilities = device.getSurfaceCapabilitiesKHR(surface);
    formats = device.getSurfaceFormatsKHR(surface);
    present_modes = device.getSurfacePresentModesKHR(surface);
}

bool check_validation_layer_support(const std::vector<const char*>& validation_layers)
{
    for (const char* layer_name : validation_layers) {
        bool layer_found = false;
        for (const auto& layer_properties : vk::enumerateInstanceLayerProperties()) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found) {
            return false;
        }
    }
    return true;
}

std::vector<const char*> get_required_extensions(bool enable_validation_layers)
{
    std::vector<const char*> extensions;
    unsigned int glfw_extension_count = 0;
    const char** glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    for (unsigned int i = 0; i < glfw_extension_count; i++) {
        extensions.push_back(glfw_extensions[i]);
    }
    if (enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    return extensions;
}

vk::Instance create_instance(
    bool enable_validation_layers, const std::vector<const char*>& validation_layers)
{
    if (enable_validation_layers && !check_validation_layer_support(validation_layers)) {
        throw std::runtime_error("Validation layer requested, but not available!");
    }

    vk::ApplicationInfo app_info(
        "Hello Triangle",
        VK_MAKE_VERSION(1, 0, 0),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0);

    auto extensions = get_required_extensions(enable_validation_layers);
    vk::InstanceCreateInfo info(
        {},
        &app_info,
        0,
        nullptr,
        static_cast<uint32_t>(extensions.size()),
        extensions.data());

    if (enable_validation_layers) {
        info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        info.ppEnabledLayerNames = validation_layers.data();
    }

    return vk::createInstance(info, nullptr);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type,
    uint64_t object,
    size_t location,
    int32_t code,
    const char* layer_prefix,
    const char* message,
    void* user_data)
{
    (void)flags;
    (void)object_type;
    (void)object;
    (void)location;
    (void)code;
    (void)layer_prefix;
    (void)user_data;
    std::cerr << "Validation layer: " << message << std::endl;
    return false;
}

VkResult create_debug_report_callback_ext(
    vk::Instance instance,
    const VkDebugReportCallbackCreateInfoEXT* info,
    VkDebugReportCallbackEXT* callback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT");

    if (func != nullptr) {
        return func(instance, info, nullptr, callback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkDebugReportCallbackEXT create_debug_callback(
    bool enable_validation_layers, vk::Instance instance)
{
    if (!enable_validation_layers) {
        return nullptr;
    }

    VkDebugReportCallbackCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    info.pfnCallback = debug_callback;

    VkDebugReportCallbackEXT callback = nullptr;
    if (create_debug_report_callback_ext(instance, &info, &callback) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to setup debug callback!");
    }
    return callback;
}

void destroy_debug_report_callback_ext(
    VkInstance instance, VkDebugReportCallbackEXT callback)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, nullptr);
    }
}

void destroy_debug_callback(vk::Instance instance, VkDebugReportCallbackEXT callback)
{
    if (callback) {
        destroy_debug_report_callback_ext(instance, callback);
    }
}

vk::SurfaceKHR create_surface(vk::Instance& instance, GLFWwindow* window)
{
    vk::SurfaceKHR surface;
    if (glfwCreateWindowSurface(
            instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
    return surface;
}

QueueFamilyIndices find_queue_families(
    vk::SurfaceKHR surface, const vk::PhysicalDevice physical_device)
{
    QueueFamilyIndices indices;
    auto queue_families = physical_device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueCount > 0 &&
            (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)) {
            indices.graphics = i;
        }

        if (queue_family.queueCount > 0 &&
            physical_device.getSurfaceSupportKHR(i, surface)) {
            indices.present = i;
        }

        if (indices.is_complete()) {
            break;
        }
        i++;
    }
    return indices;
}

bool check_device_extension_support(
    const vk::PhysicalDevice& device, const std::vector<const char*>& device_extensions)
{
    std::vector<vk::ExtensionProperties> available_extensions =
        device.enumerateDeviceExtensionProperties(nullptr);

    std::set<std::string> required_extensions(
        device_extensions.begin(), device_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

bool is_device_suitable(
    const vk::PhysicalDevice& physical_device,
    vk::SurfaceKHR& surface,
    const std::vector<const char*>& device_extensions)
{
    QueueFamilyIndices queue_family_indices =
        find_queue_families(surface, physical_device);

    bool extensions_supported =
        check_device_extension_support(physical_device, device_extensions);

    bool swap_chain_adequate = false;
    if (extensions_supported) {
        SwapChainSupportDetails details(surface, physical_device);
        swap_chain_adequate = !details.formats.empty() && !details.present_modes.empty();
    }

    bool is_suitable = queue_family_indices.is_complete() && extensions_supported &&
        swap_chain_adequate && physical_device.getFeatures().samplerAnisotropy;

    return is_suitable;
}

vk::PhysicalDevice pick_physical_device(
    vk::Instance& instance,
    vk::SurfaceKHR& surface,
    const std::vector<const char*>& device_extensions)
{
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    if (devices.size() == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    vk::PhysicalDevice physical_device;

    for (const auto& device : devices) {
        if (is_device_suitable(device, surface, device_extensions)) {
            physical_device = device;
            break;
        }
    }

    if (!physical_device) {
        throw std::runtime_error("Failed to find suitable GPU!");
    }

    return physical_device;
}

vk::Device create_logical_device(
    vk::PhysicalDevice physical_device,
    QueueFamilyIndices queue_family_indices,
    bool enable_validation_layers,
    const std::vector<const char*>& validation_layers,
    const std::vector<const char*>& device_extensions)
{
    std::vector<vk::DeviceQueueCreateInfo> queue_infos;
    std::set<int> unique_queue_families = {
        queue_family_indices.graphics, queue_family_indices.present};
    float queue_priority = 1.0f;

    for (uint32_t queue_family : unique_queue_families) {
        queue_infos.push_back({{}, queue_family, 1, &queue_priority});
    }

    vk::PhysicalDeviceFeatures device_features;
    device_features.samplerAnisotropy = true;

    vk::DeviceCreateInfo device_info(
        {},
        static_cast<uint32_t>(queue_infos.size()),
        queue_infos.data(),
        0,
        nullptr,
        static_cast<uint32_t>(device_extensions.size()),
        device_extensions.data(),
        &device_features);

    if (enable_validation_layers) {
        device_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        device_info.ppEnabledLayerNames = validation_layers.data();
    }

    vk::Device device = physical_device.createDevice(device_info, nullptr);
    return device;
}

vk::CommandPool create_command_pool(
    QueueFamilyIndices queue_family_indices, vk::Device device)
{
    vk::CommandPoolCreateInfo info(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queue_family_indices.graphics);
    vk::CommandPool command_pool = device.createCommandPool(info);
    return command_pool;
}

const std::vector<const char*> validation_layers()
{
    return {"VK_LAYER_LUNARG_standard_validation"};
}

const std::vector<const char*> device_extensions()
{
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

Device::Device(GLFWwindow* window, bool enable_validation_layers)
    : _instance(create_instance(enable_validation_layers, validation_layers())),
      _debug_callback(create_debug_callback(enable_validation_layers, _instance)),
      _surface(create_surface(_instance, window)),
      _physical_device(pick_physical_device(_instance, _surface, device_extensions())),
      _queue_family_indices(find_queue_families(_surface, _physical_device)),
      _device(create_logical_device(
          _physical_device,
          _queue_family_indices,
          enable_validation_layers,
          validation_layers(),
          device_extensions())),
      _graphics_queue(_device.getQueue(_queue_family_indices.graphics, 0)),
      _present_queue(_device.getQueue(_queue_family_indices.present, 0)),
      _command_pool(create_command_pool(_queue_family_indices, _device))
{
}

Device::~Device()
{
    _device.destroyCommandPool(_command_pool);
    _device.destroy();
    _instance.destroySurfaceKHR(_surface);
    destroy_debug_callback(_instance, _debug_callback);
    _instance.destroy();
}

uint32_t Device::find_memory_type(
    uint32_t type_filter, vk::MemoryPropertyFlags properties) const
{
    vk::PhysicalDeviceMemoryProperties memory_properties =
        _physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

vk::CommandBuffer Device::create_and_begin_command_buffer()
{
    vk::CommandBufferAllocateInfo alloc_info(
        _command_pool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer command_buffer = _device.allocateCommandBuffers(alloc_info).front();
    vk::CommandBufferBeginInfo begin_info(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
    command_buffer.begin(begin_info);
    return command_buffer;
}

void Device::flush_and_free_command_buffer(vk::CommandBuffer command_buffer)
{
    command_buffer.end();
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &command_buffer, 0, nullptr);
    _graphics_queue.submit(submitInfo, nullptr);
    _graphics_queue.waitIdle();
    static_cast<vk::Device>(_device).freeCommandBuffers(_command_pool, command_buffer);
}

vk::Format Device::find_supported_format(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates) {
        vk::FormatProperties properties = _physical_device.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear &&
            (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if (
            tiling == vk::ImageTiling::eOptimal &&
            (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format.");
}

vk::Format find_depth_attachment_optimal(Device& device)
{
    return device.find_supported_format(
        {vk::Format::eD32Sfloat,
         vk::Format::eD32SfloatS8Uint,
         vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
