#include "descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace minimal
{
    // *************** Descriptor Set Layout Builder *********************

    descriptor_set_layout::builder& descriptor_set_layout::builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count)
    {
        assert(bindings_.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings_[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<descriptor_set_layout> descriptor_set_layout::builder::build() const
    {
        return std::make_unique<descriptor_set_layout>(device_, bindings_);
    }

    // *************** Descriptor Set Layout *********************

    descriptor_set_layout::descriptor_set_layout(
        device& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device_{lveDevice}, bindings_{bindings}
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings)
            setLayoutBindings.push_back(kv.second);

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            lveDevice.get_device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor set layout!");
    }

    descriptor_set_layout::~descriptor_set_layout()
    {
        vkDestroyDescriptorSetLayout(device_.get_device(), descriptor_set_layout_, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    descriptor_pool::builder& descriptor_pool::builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count)
    {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    descriptor_pool::builder& descriptor_pool::builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags)
    {
        poolFlags = flags;
        return *this;
    }

    descriptor_pool::builder& descriptor_pool::builder::setMaxSets(uint32_t count)
    {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<descriptor_pool> descriptor_pool::builder::build() const
    {
        return std::make_unique<descriptor_pool>(device_, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    descriptor_pool::descriptor_pool(
        device& lveDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : device_{lveDevice}
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(lveDevice.get_device(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor pool!");
    }

    descriptor_pool::~descriptor_pool()
    {
        vkDestroyDescriptorPool(device_.get_device(), descriptorPool, nullptr);
    }

    bool descriptor_pool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(device_.get_device(), &allocInfo, &descriptor) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void descriptor_pool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets(
            device_.get_device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void descriptor_pool::resetPool()
    {
        vkResetDescriptorPool(device_.get_device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    descriptor_writer::descriptor_writer(descriptor_set_layout& setLayout, descriptor_pool& pool)
        : setLayout{setLayout}, pool{pool} {}

    descriptor_writer& descriptor_writer::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
    {
        assert(setLayout.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings_[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    descriptor_writer& descriptor_writer::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo)
    {
        assert(setLayout.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings_[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool descriptor_writer::build(VkDescriptorSet& set)
    {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success)
            return false;

        overwrite(set);
        return true;
    }

    void descriptor_writer::overwrite(VkDescriptorSet& set)
    {
        for (auto& write : writes)
            write.dstSet = set;

        vkUpdateDescriptorSets(pool.device_.get_device(), writes.size(), writes.data(), 0, nullptr);
    }
}
