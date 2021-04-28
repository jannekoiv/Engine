#include "descriptor_manager.h"
#include "device.h"

vk::DescriptorSetLayout create_descriptor_set_layout(
    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    vk::DescriptorSetLayoutCreateInfo info;
    info.bindingCount = static_cast<uint32_t>(bindings.size());
    info.pBindings = bindings.data();

    vk::DescriptorSetLayout layout =
        device.createDescriptorSetLayout(info, nullptr);
    return layout;
}

vk::DescriptorPool create_descriptor_pool(
    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes(bindings.size());

    for (int i = 0; i < pool_sizes.size(); i++) {
        pool_sizes[i].type = bindings[i].descriptorType;
        pool_sizes[i].descriptorCount = bindings[i].descriptorCount * max_sets;
    }

    vk::DescriptorPoolCreateInfo info{};
    info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    info.maxSets = max_sets;
    info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    info.pPoolSizes = pool_sizes.data();

    vk::DescriptorPool pool = device.createDescriptorPool(info, nullptr);
    return pool;
}

DescriptorContainer::DescriptorContainer(
    vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> bindings)
    : _device{device},
      _bindings{bindings},
      _sets_left(max_sets),
      _layout{create_descriptor_set_layout(_device, bindings)},
      _pools{create_descriptor_pool(_device, bindings)}
{
}

DescriptorContainer::~DescriptorContainer()
{
    static_cast<vk::Device>(_device).destroyDescriptorSetLayout(_layout);
    for (auto pool : _pools) {
        static_cast<vk::Device>(_device).destroyDescriptorPool(pool);
    }
}

vk::DescriptorSet DescriptorContainer::create_descriptor_set()
{
    if (_sets_left == 0) {
        std::cout << "POOL IS FULL\n";
        _pools.push_back(create_descriptor_pool(_device, _bindings));
        _sets_left = max_sets;
    }

    vk::DescriptorSetAllocateInfo info;
    info.descriptorPool = _pools.back();
    info.descriptorSetCount = 1;
    info.pSetLayouts = &_layout;

    vk::DescriptorSet set = _device.allocateDescriptorSets(info).front();
    _sets_left--;
    return set;
}

DescriptorManager::DescriptorManager(Device& device) : _device(device)
{
}

DescriptorContainer* find_descriptor_container(
    std::list<DescriptorContainer>& containers,
    std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    for (std::list<DescriptorContainer>::iterator it = containers.begin();
         it != containers.end();
         it++) {
        if (it->bindings() == bindings) {
            return &(*it);
        }
    }
    return nullptr;
}

DescriptorSet DescriptorManager::create_descriptor_set(
    std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    auto container = find_descriptor_container(_containers, bindings);

    if (!container) {
        std::cout << "Container not found\n";
        container = &_containers.emplace_back(_device, bindings);
    }

    return DescriptorSet{
        _device,
        container->bindings(),
        container->create_descriptor_set(),
        container->layout()};
}
