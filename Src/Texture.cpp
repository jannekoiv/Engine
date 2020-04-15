
#include "../Include/Texture.h"
#include "../Include/Base.h"
#include "../Include/Buffer.h"
#include "../Include/Device.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

Texture::Texture(Texture&& rhs)
    : mDevice{rhs.mDevice},
      mType{rhs.mType},
      mLayerCount{rhs.mLayerCount},
      mExtent{rhs.mExtent},
      mFormat{rhs.mFormat},
      mImage{rhs.mImage},
      mMemory{rhs.mMemory},
      mImageView{rhs.mImageView},
      mSampler{rhs.mSampler}
{
    rhs.mImage = nullptr;
    rhs.mMemory = nullptr;
    rhs.mImageView = nullptr;
    rhs.mSampler = nullptr;
}

vk::ImageType imageType(vk::ImageViewType type)
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

vk::ImageCreateFlags imageCreateFlags(vk::ImageViewType viewType)
{
    if (viewType == vk::ImageViewType::eCube) {
        return vk::ImageCreateFlagBits::eCubeCompatible;
    } else {
        return {};
    }
}

static vk::Image createImage(
    vk::Device device,
    vk::ImageViewType viewType,
    int layerCount,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.arrayLayers = layerCount;
    imageInfo.extent = vk::Extent3D{extent.width, extent.height, 1};
    imageInfo.flags = imageCreateFlags(viewType);
    imageInfo.format = format;
    imageInfo.imageType = imageType(viewType);
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.mipLevels = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;

    vk::Image image = device.createImage(imageInfo, nullptr);
    return image;
}

vk::DeviceMemory allocateAndBindMemory(Device& device, vk::Image image, vk::MemoryPropertyFlags memoryProperties)
{
    vk::MemoryRequirements memRequirements = static_cast<vk::Device>(device).getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size, device.findMemoryType(memRequirements.memoryTypeBits, memoryProperties));

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

vk::ImageAspectFlags aspectMaskFromFormat(vk::Format format)
{
    if (isDepthFormat(format)) {
        return vk::ImageAspectFlagBits::eDepth;
    } else {
        return vk::ImageAspectFlagBits::eColor;
    }
}

vk::ImageView createImageView(vk::Device device, vk::ImageViewType type, vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = type;
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
    uint32_t layerCount,
    vk::Extent3D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memoryProperties,
    vk::SamplerAddressMode addressMode)
    : mDevice{device},
      mType{type},
      mLayerCount{layerCount},
      mExtent{extent},
      mFormat{format},
      mImage{createImage(mDevice, mType, layerCount, mExtent, mFormat, tiling, usage)},
      mMemory{allocateAndBindMemory(mDevice, mImage, memoryProperties)},
      mImageView{createImageView(mDevice, mType, mImage, mFormat)},
      mSampler{createSampler(device, addressMode)}
{
}

Texture::~Texture()
{
    static_cast<vk::Device>(mDevice).destroySampler(mSampler);
    static_cast<vk::Device>(mDevice).destroyImageView(mImageView);
    static_cast<vk::Device>(mDevice).freeMemory(mMemory);
    static_cast<vk::Device>(mDevice).destroyImage(mImage);
}

void Texture::transitionLayout(
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer externalCommandBuffer)
{
    vk::CommandBuffer commandBuffer = externalCommandBuffer;
    if (!commandBuffer) {
        commandBuffer = mDevice.createAndBeginCommandBuffer();
    }

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = mImage;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = mLayerCount;

    if (isDepthFormat(mFormat)) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (hasStencilComponent(mFormat)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = {};
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (
        oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
        newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead, srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (
        oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;
        srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (oldLayout == vk::ImageLayout::ePresentSrcKHR && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead, srcStage = vk::PipelineStageFlagBits::eAllGraphics;
        dstStage = vk::PipelineStageFlagBits::eAllGraphics;
    } else if (oldLayout == vk::ImageLayout::ePresentSrcKHR && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal &&
        newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (
        oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        std::cout << "TRANSFERING BACK\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        oldLayout == vk::ImageLayout::eTransferSrcOptimal &&
        newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        std::cout << "TRANSFERING BACK\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR) {
        std::cout << "TRANSFERING\n";
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (
        oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        std::cout << "UNDEFINED TO DEPTH STENCIL\n";
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else if (oldLayout == newLayout) {
    } else {
        std::cout << "Unsupported layout transition.\n";
        throw std::invalid_argument("Unsupported layout transition.");
    }

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, barrier);

    if (!externalCommandBuffer) {
        mDevice.flushAndFreeCommandBuffer(commandBuffer);
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
