#pragma once

#include <memory>

#include "../rendering/vulkan/VulkanDevice.hpp"
#include "FrameInfo.hpp"
#include "../rendering/RenderSubsystem.hpp"
#include "rendering/vulkan/VulkanPipeline.hpp"

namespace Minimal {
    class SimpleRendererSystem final : public RenderSubsystem {
    public:
        SimpleRendererSystem(ECSCoordinator &ecs, VulkanDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        ~SimpleRendererSystem() override;

        SimpleRendererSystem(const SimpleRendererSystem &) = delete;

        SimpleRendererSystem &operator=(const SimpleRendererSystem &) = delete;

        void update(FrameInfo &frameInfo) override;

        void render(FrameInfo &frameInfo) override;

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

        void createPipeline(VkRenderPass renderPass);

        VulkanDevice &m_device;

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}
