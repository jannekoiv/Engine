#define STB_IMAGE_IMPLEMENTATION

#include "../Include/Texture.h"
#include "../Include/Base.h"
#include "../Include/Device.h"
#include "../Include/Buffer.h"
#include "stb_image.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

static vk::Image createImage(
    vk::Device device,
    int layerCount,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.arrayLayers = layerCount;
    imageInfo.extent = vk::Extent3D{extent.width, extent.height, 1};
    imageInfo.flags = {};
    imageInfo.format = format;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.mipLevels = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;

    vk::Image image = device.createImage(imageInfo, nullptr);
    return image;
}

vk::DeviceMemory allocateAndBindMemory(
    Device& device, vk::Image image, vk::MemoryPropertyFlags memoryProperties)
{
    vk::MemoryRequirements memRequirements =
        static_cast<vk::Device>(device).getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        device.findMemoryType(memRequirements.memoryTypeBits, memoryProperties));

    vk::DeviceMemory memory = static_cast<vk::Device>(device).allocateMemory(allocInfo, nullptr);
    static_cast<vk::Device>(device).bindImageMemory(image, memory, 0);
    return memory;
}

bool isDepthFormat(vk::Format format)
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

vk::ImageAspectFlagBits aspectMaskFromFormat(vk::Format format)
{
    if (isDepthFormat(format)) {
        return vk::ImageAspectFlagBits::eDepth;
    } else {
        return vk::ImageAspectFlagBits::eColor;
    }
}

vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectMaskFromFormat(format);
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView = device.createImageView(viewInfo, nullptr);
    return imageView;
}

vk::Sampler createSampler(vk::Device device, vk::SamplerAddressMode addressMode)
{
    vk::SamplerCreateInfo samplerInfo{};

    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;

    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;

    samplerInfo.anisotropyEnable = true;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
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
    int layerCount,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memoryProperties,
    vk::SamplerAddressMode addressMode)
    : mDevice{device},
      mExtent{extent},
      mFormat{format},
      mLayout{vk::ImageLayout::eUndefined},
      mImage{createImage(mDevice, layerCount, mExtent, mFormat, tiling, usage)},
      mMemory{allocateAndBindMemory(mDevice, mImage, memoryProperties)},
      mView{createImageView(mDevice, mImage, mFormat)},
      mSampler{createSampler(device, addressMode)}
{
}

Texture::~Texture()
{
    //    static_cast<vk::Device>(mDevice).freeMemory(mMemory);
    //    static_cast<vk::Device>(mDevice).destroyImage(mImage);
}

void Texture::transitionLayout(vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = mDevice.createAndBeginCommandBuffer();

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = mLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = mImage;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (mLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        mLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (mLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = {};
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        mLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        mLayout == vk::ImageLayout::eTransferDstOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        mLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        mLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        mLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        mLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        mLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        mLayout == vk::ImageLayout::eColorAttachmentOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        mLayout == vk::ImageLayout::ePresentSrcKHR &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        mLayout == vk::ImageLayout::ePresentSrcKHR &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        mLayout == vk::ImageLayout::eColorAttachmentOptimal &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        mLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        std::cout << "TRANSFERING BACK\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        mLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::ePresentSrcKHR) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        mLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else {
        std::cout << "Unsupported layout transition.\n";
        throw std::invalid_argument("Unsupported layout transition.");
    }

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (hasStencilComponent(mFormat)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, barrier);

    mDevice.flushAndFreeCommandBuffer(commandBuffer);
}

Texture createTextureFromFile(Device& device, std::string filename)
{
    const int bytesPerPixel = 4;
    int width = 0;
    int height = 0;
    int channelCount = 0;

    stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channelCount, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    vk::DeviceSize imageSize = width * height * bytesPerPixel;

    Buffer stagingBuffer(
        device,
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = stagingBuffer.mapMemory();
    memcpy(data, pixels, imageSize);
    stagingBuffer.unmapMemory();
    stbi_image_free(pixels);

    Texture texture{
        device,
        1,
        vk::Extent3D(width, height, 1),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SamplerAddressMode::eRepeat};

    texture.transitionLayout(vk::ImageLayout::eTransferDstOptimal);
    stagingBuffer.copyToImage(texture);
    texture.transitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    return texture;
}
