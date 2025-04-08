#pragma once

#include <memory>

#include "../rendering/vulkan/VulkanDevice.hpp"
#include "FrameInfo.hpp"
#include "System.hpp"
#include "rendering/vulkan/VulkanPipeline.hpp"

namespace Minimal {
    class PointLightSystem : public System {
    public:
        PointLightSystem(ECSCoordinator& ecs, VulkanDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        ~PointLightSystem() override;

        PointLightSystem(const PointLightSystem &) = delete;

        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void update(FrameInfo &frameInfo);

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

        void createPipeline(VkRenderPass renderPass);

        VulkanDevice &m_device;

        std::unique_ptr<VulkanPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}
