#pragma once

#include <memory>

#include "../rendering/vulkan/VulkanDevice.hpp"
#include "FrameInfo.hpp"
#include "rendering/vulkan/VulkanPipeline.hpp"

namespace Minimal {
    class SimpleRendererSystem {
    public:
        SimpleRendererSystem(VulkanDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        ~SimpleRendererSystem();

        SimpleRendererSystem(const SimpleRendererSystem &) = delete;

        SimpleRendererSystem &operator=(const SimpleRendererSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

        void createPipeline(VkRenderPass renderPass);

        VulkanDevice &m_device;

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}
