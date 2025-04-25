#pragma once

#include <memory>

#include "../rendering/vulkan/VulkanDevice.hpp"
#include "FrameInfo.hpp"
#include "System.hpp"
#include "rendering/vulkan/VulkanPipeline.hpp"

namespace Minimal {
    class SimpleRendererSystem {
    public:
        SimpleRendererSystem(ECSCoordinator &ecs, VulkanDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        ~SimpleRendererSystem();

        SimpleRendererSystem(const SimpleRendererSystem &) = delete;

        SimpleRendererSystem &operator=(const SimpleRendererSystem &) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

        void createPipeline(VkRenderPass renderPass);

        ECSCoordinator &m_ecs;
        VulkanDevice &m_device;

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}
