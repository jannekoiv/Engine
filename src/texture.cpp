
#include "texture.h"
#include "base.h"
#include "buffer.h"
#include "device.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

Texture::Texture(Texture&& rhs)
    : _device{rhs._device},
      _type{rhs._type},
      _layer_count{rhs._layer_count},
      _extent{rhs._extent},
      _format{rhs._format},
      _image{rhs._image},
      _memory{rhs._memory},
      _image_view{rhs._image_view},
      _sampler{rhs._sampler}
{
    rhs._image = nullptr;
    rhs._memory = nullptr;
    rhs._image_view = nullptr;
    rhs._sampler = nullptr;
}

vk::ImageType image_type(vk::ImageViewType type)
{
    if (type == vk::ImageViewType::e1D) {
        return vk::ImageType::e1D;
    } else if (type == vk::ImageViewType::e1DArray) {
        return vk::ImageType::e1D;
    } else if (type == vk::ImageViewType::e2D) {
        return vk::ImageType::e2D;
    } else if (type == vk::ImageViewType::e2DArray) {
        return vk::ImageType::e2D;
    } else if (type == vk::ImageViewType::e3D) {
        return vk::ImageType::e3D;
    } else if (type == vk::ImageViewType::eCube) {
        return vk::ImageType::e2D;
    } else if (type == vk::ImageViewType::eCubeArray) {
        return vk::ImageType::e2D;
    } else {
        return vk::ImageType::e2D;
    }
}

vk::ImageCreateFlags image_create_flags(vk::ImageViewType view_type)
{
    if (view_type == vk::ImageViewType::eCube) {
        return vk::ImageCreateFlagBits::eCubeCompatible;
    } else {
        return {};
    }
}

static vk::Image create_image(
    vk::Device device,
    vk::ImageViewType view_type,
    int layer_count,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage)
{
    vk::ImageCreateInfo image_info{};
    image_info.arrayLayers = layer_count;
    image_info.extent = vk::Extent3D{extent.width, extent.height, 1};
    image_info.flags = image_create_flags(view_type);
    image_info.format = format;
    image_info.imageType = image_type(view_type);
    image_info.initialLayout = vk::ImageLayout::eUndefined;
    image_info.mipLevels = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.sharingMode = vk::SharingMode::eExclusive;
    image_info.tiling = tiling;
    image_info.usage = usage;

    vk::Image image = device.createImage(image_info, nullptr);
    return image;
}

vk::DeviceMemory allocate_and_bind_memory(Device& device, vk::Image image, vk::MemoryPropertyFlags memory_properties)
{
    vk::MemoryRequirements memory_requirements = static_cast<vk::Device>(device).getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo alloc_info(
        memory_requirements.size, device.find_memory_type(memory_requirements.memoryTypeBits, memory_properties));

    vk::DeviceMemory memory = static_cast<vk::Device>(device).allocateMemory(alloc_info, nullptr);
    static_cast<vk::Device>(device).bindImageMemory(image, memory, 0);
    return memory;
}

bool is_depth_format(vk::Format format)
{
    std::vector<vk::Format> formats = {
        vk::Format::eD16Unorm,
        vk::Format::eX8D24UnormPack32,
        vk::Format::eD32Sfloat,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD32SfloatS8Uint};
    return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}

vk::ImageAspectFlags aspect_mask_from_format(vk::Format format)
{
    if (is_depth_format(format)) {
        return vk::ImageAspectFlagBits::eDepth;
    } else {
        return vk::ImageAspectFlagBits::eColor;
    }
}

vk::ImageView create_image_view(vk::Device device, vk::ImageViewType type, vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo view_info;
    view_info.image = image;
    view_info.viewType = type;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_mask_from_format(format);
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    vk::ImageView image_view = device.createImageView(view_info, nullptr);
    return image_view;
}

vk::Sampler create_sampler(vk::Device device, vk::SamplerAddressMode address_mode)
{
    vk::SamplerCreateInfo samplerInfo{};

    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;

    samplerInfo.addressModeU = address_mode;
    samplerInfo.addressModeV = address_mode;
    samplerInfo.addressModeW = address_mode;

    samplerInfo.anisotropyEnable = true;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    samplerInfo.unnormalizedCoordinates = false;

    samplerInfo.compareEnable = false;
    samplerInfo.compareOp = vk::CompareOp::eAlways;

    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    vk::Sampler sampler = device.createSampler(samplerInfo);
    return sampler;
}

Texture::Texture(
    Device& device,
    vk::ImageViewType type,
    uint32_t layer_count,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memory_properties,
    vk::SamplerAddressMode address_mode)
    : _device{device},
      _type{type},
      _layer_count{layer_count},
      _extent{extent},
      _format{format},
      _image{create_image(_device, _type, layer_count, _extent, _format, tiling, usage)},
      _memory{allocate_and_bind_memory(_device, _image, memory_properties)},
      _image_view{create_image_view(_device, _type, _image, _format)},
      _sampler{create_sampler(device, address_mode)}
{
}

Texture::~Texture()
{
    static_cast<vk::Device>(_device).destroySampler(_sampler);
    static_cast<vk::Device>(_device).destroyImageView(_image_view);
    static_cast<vk::Device>(_device).freeMemory(_memory);
    static_cast<vk::Device>(_device).destroyImage(_image);
}

void Texture::transition_layout(
    vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::CommandBuffer external_command_buffer)
{
    vk::CommandBuffer command_buffer = external_command_buffer;
    if (!command_buffer) {
        command_buffer = _device.create_and_begin_command_buffer();
    }

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = _image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = _layer_count;

    if (is_depth_format(_format)) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (has_stencil_component(_format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    vk::PipelineStageFlags src_stage;
    vk::PipelineStageFlags dst_stage;

    if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = {};
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        old_layout == vk::ImageLayout::eTransferSrcOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eFragmentShader;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eFragmentShader;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        old_layout == vk::ImageLayout::eShaderReadOnlyOptimal && new_layout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eFragmentShader;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        old_layout == vk::ImageLayout::eShaderReadOnlyOptimal && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eFragmentShader;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        old_layout == vk::ImageLayout::eShaderReadOnlyOptimal && new_layout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        src_stage = vk::PipelineStageFlagBits::eAllGraphics;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        old_layout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eAllGraphics;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (old_layout == vk::ImageLayout::eShaderReadOnlyOptimal && new_layout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        src_stage = vk::PipelineStageFlagBits::eAllGraphics;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        old_layout == vk::ImageLayout::eColorAttachmentOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead, src_stage = vk::PipelineStageFlagBits::eAllGraphics;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        old_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal &&
        new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;
        src_stage = vk::PipelineStageFlagBits::eAllGraphics;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (old_layout == vk::ImageLayout::ePresentSrcKHR && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead, src_stage = vk::PipelineStageFlagBits::eAllGraphics;
        dst_stage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (old_layout == vk::ImageLayout::ePresentSrcKHR && new_layout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        old_layout == vk::ImageLayout::eColorAttachmentOptimal && new_layout == vk::ImageLayout::eTransferSrcOptimal) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        old_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal &&
        new_layout == vk::ImageLayout::eTransferSrcOptimal) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        old_layout == vk::ImageLayout::eTransferSrcOptimal && new_layout == vk::ImageLayout::eColorAttachmentOptimal) {
        std::cout << "TRANSFERING BACK\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        old_layout == vk::ImageLayout::eTransferSrcOptimal &&
        new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        std::cout << "TRANSFERING BACK\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (old_layout == vk::ImageLayout::eTransferSrcOptimal && new_layout == vk::ImageLayout::ePresentSrcKHR) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        std::cout << "UNDEFINED TO DEPTH STENCIL\n";
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else if (old_layout == new_layout) {
    } else {
        std::cout << "Unsupported layout transition.\n";
        throw std::invalid_argument("Unsupported layout transition.");
    }

    command_buffer.pipelineBarrier(src_stage, dst_stage, {}, nullptr, nullptr, barrier);

    if (!external_command_buffer) {
        _device.flush_and_free_command_buffer(command_buffer);
    }
}

//Texture createTextureFromFile(Device& device, std::string filename, vk::SamplerAddressMode addressMode)
//{
//    const int bytesPerPixel = 4;
//    int width = 0;
//    int height = 0;
//    int channelCount = 0;
//
//    stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channelCount, STBI_rgb_alpha);
//    if (!pixels) {
//        throw std::runtime_error("Failed to load texture image!");
//    }
//
//    vk::DeviceSize imageSize = width * height * bytesPerPixel;
//
//    Buffer stagingBuffer(
//        device,
//        imageSize,
//        vk::BufferUsageFlagBits::eTransferSrc,
//        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//    void* data = stagingBuffer.mapMemory();
//    memcpy(data, pixels, imageSize);
//    stagingBuffer.unmapMemory();
//    stbi_image_free(pixels);
//
//    Texture texture{
//        device,
//        vk::ImageViewType::e2D,
//        1,
//        vk::Extent3D(width, height, 1),
//        vk::Format::eR8G8B8A8Unorm,
//        vk::ImageTiling::eOptimal,
//        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled,
//        vk::MemoryPropertyFlagBits::eDeviceLocal,
//        addressMode};
//
//    texture.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
//    stagingBuffer.copyToTexture(texture, 0);
//    texture.transitionLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
//
//    return texture;
//}

//Texture createCubeTextureFromFile(Device& device, std::string filename)
//{
//    std::array<std::string, 6> filenames = {
//        "d:/skybox/right.jpg",
//        "d:/skybox/left.jpg",
//        "d:/skybox/top.jpg",
//        "d:/skybox/bottom.jpg",
//        "d:/skybox/front.jpg",
//        "d:/skybox/back.jpg"};
//
//    std::vector<Buffer> stagingBuffers{};
//
//    int width = 0;
//    int height = 0;
//
//    for (int face = 0; face < 6; face++) {
//        const int bytesPerPixel = 4;
//        int channelCount = 0;
//
//        stbi_uc* pixels = stbi_load(filenames[face].data(), &width, &height, &channelCount, STBI_rgb_alpha);
//        if (!pixels) {
//            throw std::runtime_error("Failed to load texture image!");
//        }
//
//        vk::DeviceSize imageSize = width * height * bytesPerPixel;
//
//        stagingBuffers.emplace_back(
//            device,
//            imageSize,
//            vk::BufferUsageFlagBits::eTransferSrc,
//            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//        Buffer& stagingBuffer = stagingBuffers.back();
//
//        void* data = stagingBuffer.mapMemory();
//        memcpy(data, pixels, imageSize);
//        stagingBuffer.unmapMemory();
//        stbi_image_free(pixels);
//    }
//
//    Texture texture{
//        device,
//        vk::ImageViewType::eCube,
//        6,
//        vk::Extent3D(width, height, 1),
//        vk::Format::eR8G8B8A8Unorm,
//        vk::ImageTiling::eOptimal,
//        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled,
//        vk::MemoryPropertyFlagBits::eDeviceLocal,
//        vk::SamplerAddressMode::eClampToEdge};
//
//    texture.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
//    for (int face = 0; face < 6; face++) {
//        stagingBuffers[face].copyToTexture(texture, face);
//    }
//    texture.transitionLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
//
//    return texture;
//}
