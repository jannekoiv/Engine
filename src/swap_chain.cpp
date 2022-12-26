#include "includes.h"

vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats)
{
    if (available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined) {
        vk::SurfaceFormatKHR format;
        format.format = vk::Format::eB8G8R8A8Unorm;
        format.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        return format;
    }
    for (const auto& format : available_formats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return available_formats[0];
}

vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR> available_modes)
{
    vk::PresentModeKHR preferred_mode = vk::PresentModeKHR::eFifo;
    for (const auto& mode : available_modes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        } else if (mode == vk::PresentModeKHR::eImmediate) {
            preferred_mode = mode;
        }
    }
    return preferred_mode;
}

uint32_t choose_swap_image_count(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }
    return image_count;
}

static vk::ImageView create_image_view(vk::Device device, vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo info;
    info.image = image;
    info.viewType = vk::ImageViewType::e2D;
    info.format = format;
    info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;

    vk::ImageView view = device.createImageView(info, nullptr);
    return view;
}

SwapChain::SwapChain(Device& device) : _device{device}
{
    SwapChainSupportDetails swap_chain_support_details(_device.surface(), _device.physical_device());
    auto surface_format = choose_swap_surface_format(swap_chain_support_details.formats);
    _format = surface_format.format;
    _extent = swap_chain_support_details.capabilities.currentExtent;
    auto present_mode = choose_swap_present_mode(swap_chain_support_details.present_modes);
    auto image_count = choose_swap_image_count(swap_chain_support_details.capabilities);

    vk::SwapchainCreateInfoKHR create_info;
    create_info.surface = _device.surface();
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = _extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    create_info.preTransform = swap_chain_support_details.capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = present_mode;
    create_info.clipped = true;
    create_info.oldSwapchain = nullptr;

    int graphics_family = _device.queue_family_indices().graphics;
    int present_family = _device.queue_family_indices().present;
    if (graphics_family != present_family) {
        create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        create_info.queueFamilyIndexCount = 2;
        uint32_t indices[] = {static_cast<uint32_t>(graphics_family), static_cast<uint32_t>(present_family)};
        create_info.pQueueFamilyIndices = indices;
    } else {
        create_info.imageSharingMode = vk::SharingMode::eExclusive;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    _swap_chain = static_cast<vk::Device>(_device).createSwapchainKHR(create_info, nullptr);
    auto images = static_cast<vk::Device>(_device).getSwapchainImagesKHR(_swap_chain);
    std::cout << "swapchain images size " << images.size() << "\n";

    for (size_t i = 0; i < images.size(); i++) {
        _images.push_back(images[i]);
        _image_views.push_back(create_image_view(_device, images[i], _format));
    }
}

SwapChain::~SwapChain()
{
    for (auto view : _image_views) {
        static_cast<vk::Device>(_device).destroyImageView(view);
    }
    static_cast<vk::Device>(_device).destroySwapchainKHR(_swap_chain);
}

vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D& extent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        std::cout << "FORCED\n";
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actual_extent = extent;
        actual_extent.width = std::max(
            capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
        actual_extent.height = std::max(
            capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));
        return actual_extent;
    }
}
