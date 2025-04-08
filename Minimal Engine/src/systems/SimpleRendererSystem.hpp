#pragma once

#include <memory>

#include "../rendering/vulkan/VulkanDevice.hpp"
#include "FrameInfo.hpp"
#include "System.hpp"
#include "rendering/vulkan/VulkanPipeline.hpp"

namespace Minimal {
    class SimpleRendererSystem : public System {
    public:
        SimpleRendererSystem(ECSCoordinator &ecs, VulkanDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        ~SimpleRendererSystem();

        SimpleRendererSystem(const SimpleRendererSystem &) = delete;

        SimpleRendererSystem &operator=(const SimpleRendererSystem &) = delete;

        void render(FrameInfo &frameInfo);

        void update(FrameInfo &frameInfo) override;

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

        void createPipeline(VkRenderPass renderPass);

        VulkanDevice &m_device;

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}
