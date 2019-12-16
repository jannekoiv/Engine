
#include "../Include/Material.h"

static std::vector<Framebuffer> createFramebuffers(
    Device& device, SwapChain& swapChain, Image& depthImage, RenderPass& renderPass)
{
    std::vector<Framebuffer> frameBuffers;
    for (int i = 0; i < swapChain.imageCount(); i++) {
        frameBuffers.emplace_back(
            device, swapChain.imageView(i), depthImage.view(), swapChain.extent(), renderPass);
    }
    return frameBuffers;
}

Material::Material(
    Device& device,
    SwapChain& swapChain,
    Image& depthImage,
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions,
    vk::DescriptorSetLayout descriptorSetLayout,
    std::string vertexShaderFilename,
    std::string fragmentShaderFilename)
    : mBindingDescription{bindingDescription},
      mAttributeDescriptions{attributeDescriptions},
      mDescriptorSetLayout{descriptorSetLayout},
      mRenderPass{device, swapChain.format(), vk::AttachmentLoadOp::eLoad},
      mFramebuffers{createFramebuffers(device, swapChain, depthImage, mRenderPass)},
      mPipeline{
          device,
          mBindingDescription,
          mAttributeDescriptions,
          mDescriptorSetLayout,
          vertexShaderFilename,
          fragmentShaderFilename,
          swapChain.extent(),
          mRenderPass}
{
}
