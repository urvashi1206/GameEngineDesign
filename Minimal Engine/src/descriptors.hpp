#pragma once

#include "rendering/vulkan/vulkan_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace minimal
{
    class descriptor_set_layout
    {
    public:
        class builder
        {
        public:
            builder(vulkan_device& device) : device_{device} {}

            builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1
            );
            std::unique_ptr<descriptor_set_layout> build() const;

        private:
            vulkan_device& device_;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        descriptor_set_layout(
            vulkan_device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~descriptor_set_layout();
        descriptor_set_layout(const descriptor_set_layout&) = delete;
        descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptor_set_layout_; }

    private:
        vulkan_device& device_;
        VkDescriptorSetLayout descriptor_set_layout_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

        friend class descriptor_writer;
    };

    class descriptor_pool
    {
    public:
        class builder
        {
        public:
            builder(vulkan_device& device) : device_{device} {}

            builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            builder& setMaxSets(uint32_t count);
            std::unique_ptr<descriptor_pool> build() const;

        private:
            vulkan_device& device_;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        descriptor_pool(
            vulkan_device& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~descriptor_pool();
        descriptor_pool(const descriptor_pool&) = delete;
        descriptor_pool& operator=(const descriptor_pool&) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        vulkan_device& device_;
        VkDescriptorPool descriptorPool;

        friend class descriptor_writer;
    };

    class descriptor_writer
    {
    public:
        descriptor_writer(descriptor_set_layout& setLayout, descriptor_pool& pool);

        descriptor_writer& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        descriptor_writer& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        descriptor_set_layout& setLayout;
        descriptor_pool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
}
