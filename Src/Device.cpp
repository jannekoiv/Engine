#include "../Include/Device.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <vulkan/vulkan.hpp>

SwapChainSupportDetails::SwapChainSupportDetails(vk::SurfaceKHR surface, vk::PhysicalDevice device)
{
    capabilities = device.getSurfaceCapabilitiesKHR(surface);
    formats = device.getSurfaceFormatsKHR(surface);
    presentModes = device.getSurfacePresentModesKHR(surface);
}

bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : vk::enumerateInstanceLayerProperties()) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

std::vector<const char*> getRequiredExtensions(bool enableValidationLayers)
{
    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (unsigned int i = 0; i < glfwExtensionCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    return extensions;
}

vk::Instance createInstance(bool enableValidationLayers, const std::vector<const char*>& validationLayers)
{
    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
        throw std::runtime_error("Validation layer requested, but not available!");
    }

    vk::ApplicationInfo appInfo(
        "Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

    auto extensions = getRequiredExtensions(enableValidationLayers);
    vk::InstanceCreateInfo createInfo(
        {}, &appInfo, 0, nullptr, static_cast<uint32_t>(extensions.size()), extensions.data());

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    return vk::createInstance(createInfo, nullptr);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* message,
    void* userData)
{
    (void)flags;
    (void)objectType;
    (void)object;
    (void)location;
    (void)code;
    (void)layerPrefix;
    (void)userData;
    std::cerr << "Validation layer: " << message << std::endl;
    return false;
}

VkResult createDebugReportCallbackEXT(
    vk::Instance instance, const VkDebugReportCallbackCreateInfoEXT* createInfo, VkDebugReportCallbackEXT* callback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

    if (func != nullptr) {
        return func(instance, createInfo, nullptr, callback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkDebugReportCallbackEXT createDebugCallback(bool enableValidationLayers, vk::Instance instance)
{
    if (!enableValidationLayers) {
        return nullptr;
    }

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    VkDebugReportCallbackEXT callback = nullptr;
    if (createDebugReportCallbackEXT(instance, &createInfo, &callback) != VK_SUCCESS) {
        throw std::runtime_error("Failed to setup debug callback!");
    }
    return callback;
}

void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, nullptr);
    }
}

void destroyDebugCallback(vk::Instance instance, VkDebugReportCallbackEXT callback)
{
    if (callback) {
        destroyDebugReportCallbackEXT(instance, callback);
    }
}

vk::SurfaceKHR createSurface(vk::Instance& instance, GLFWwindow* window)
{
    vk::SurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
    return surface;
}

QueueFamilyIndices findQueueFamilies(vk::SurfaceKHR surface, const vk::PhysicalDevice physicalDevice)
{
    QueueFamilyIndices indices;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
            indices.graphics = i;
        }

        if (queueFamily.queueCount > 0 && physicalDevice.getSurfaceSupportKHR(i, surface)) {
            indices.present = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& deviceExtensions)
{
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties(nullptr);

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

bool isDeviceSuitable(
    const vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(surface, physicalDevice);

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails details(surface, physicalDevice);
        swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
    }

    bool isSuitable = queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate &&
        physicalDevice.getFeatures().samplerAnisotropy;

    return isSuitable;
}

vk::PhysicalDevice pickPhysicalDevice(
    vk::Instance& instance, vk::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    if (devices.size() == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    vk::PhysicalDevice physicalDevice;

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, surface, deviceExtensions)) {
            physicalDevice = device;
            break;
        }
    }

    if (!physicalDevice) {
        throw std::runtime_error("Failed to find suitable GPU!");
    }

    return physicalDevice;
}

vk::Device createLogicalDevice(
    vk::PhysicalDevice physicalDevice,
    QueueFamilyIndices queueFamilyIndices,
    bool enableValidationLayers,
    const std::vector<const char*>& validationLayers,
    const std::vector<const char*>& deviceExtensions)
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {queueFamilyIndices.graphics, queueFamilyIndices.present};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({{}, queueFamily, 1, &queuePriority});
    }

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = true;

    vk::DeviceCreateInfo createInfo(
        {},
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data(),
        0,
        nullptr,
        static_cast<uint32_t>(deviceExtensions.size()),
        deviceExtensions.data(),
        &deviceFeatures);

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    vk::Device device = physicalDevice.createDevice(createInfo, nullptr);
    return device;
}

vk::CommandPool createCommandPool(QueueFamilyIndices queueFamilyIndices, vk::Device device)
{
    vk::CommandPoolCreateInfo commandPoolInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndices.graphics);
    vk::CommandPool commandPool = device.createCommandPool(commandPoolInfo);
    return commandPool;
}

const std::vector<const char*> validationLayers()
{
    return {"VK_LAYER_LUNARG_standard_validation"};
}

const std::vector<const char*> deviceExtensions()
{
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

Device::Device(GLFWwindow* window, bool enableValidationLayers)
    : mInstance(createInstance(enableValidationLayers, validationLayers())),
      mDebugCallback(createDebugCallback(enableValidationLayers, mInstance)),
      mSurface(createSurface(mInstance, window)),
      mPhysicalDevice(pickPhysicalDevice(mInstance, mSurface, deviceExtensions())),
      mQueueFamilyIndices(findQueueFamilies(mSurface, mPhysicalDevice)),
      mDevice(createLogicalDevice(
          mPhysicalDevice, mQueueFamilyIndices, enableValidationLayers, validationLayers(), deviceExtensions())),
      mGraphicsQueue(mDevice.getQueue(mQueueFamilyIndices.graphics, 0)),
      mPresentQueue(mDevice.getQueue(mQueueFamilyIndices.present, 0)),
      mCommandPool(createCommandPool(mQueueFamilyIndices, mDevice))
{
}

Device::~Device()
{
    mDevice.destroyCommandPool(mCommandPool);
    mDevice.destroy();
    mInstance.destroySurfaceKHR(mSurface);
    destroyDebugCallback(mInstance, mDebugCallback);
    mInstance.destroy();
}

uint32_t Device::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    vk::PhysicalDeviceMemoryProperties memProperties = mPhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

vk::CommandBuffer Device::createAndBeginCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer commandBuffer = mDevice.allocateCommandBuffers(allocInfo).front();
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void Device::flushAndFreeCommandBuffer(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr);
    mGraphicsQueue.submit(submitInfo, nullptr);
    mGraphicsQueue.waitIdle();
    static_cast<vk::Device>(mDevice).freeCommandBuffers(mCommandPool, commandBuffer);
}

vk::Format Device::findSupportedFormat(
    const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates) {
        vk::FormatProperties properties = mPhysicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format.");
}

vk::Format findDepthAttachmentFormat(Device& device)
{
    return device.findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
