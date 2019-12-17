
#include "../Include/Material.h"

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
      mFramebufferSet{device, swapChain, depthImage, vk::AttachmentLoadOp::eLoad},
      mPipeline{
          device,
          mBindingDescription,
          mAttributeDescriptions,
          mDescriptorSetLayout,
          vertexShaderFilename,
          fragmentShaderFilename,
          swapChain.extent(),
          mFramebufferSet.renderPass()}
{
}
