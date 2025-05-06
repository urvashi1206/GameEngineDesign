#pragma once

#include <memory>

#include "VulkanDevice.hpp"
#include "VulkanSwapChain.hpp"
#include "../../Window.hpp"
#include "systems/System.hpp"

namespace Minimal {
    class CameraSystem;
    class PointLightSystem;
    class SimpleRendererSystem;
    class VulkanDescriptorPool;

    class VulkanRenderer : public System {
    public:
        VulkanRenderer(ECSCoordinator &ecs, Window &window, VulkanDevice &device);

        ~VulkanRenderer();

        VulkanRenderer(const VulkanRenderer &) = delete;

        VulkanRenderer &operator=(const VulkanRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }

        float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }

        bool isFrameInProgress() const { return m_isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(m_isFrameStarted && "Cannot get command buffer while frame is not in progress");
            return m_commandBuffers[m_currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(m_isFrameStarted && "Cannot get frame index while frame is not in progress");
            return m_currentFrameIndex;
        }

        void initialize();

        void update(float deltaTime) override;

        void shutdown();

    private:
        void createCommandBuffers();

        void freeCommandBuffers();

        void recreateSwapChain();

        VkCommandBuffer beginFrame();

        void endFrame();

        void beingSwapChainRenderPass(VkCommandBuffer commandBuffer);

        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        Window &m_window;
        VulkanDevice &m_device;
        std::unique_ptr<VulkanSwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex;
        int m_currentFrameIndex{0};
        bool m_isFrameStarted{false};

        std::vector<std::unique_ptr<VulkanBuffer> > m_uboBuffers{VulkanSwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_globalDescriptorSets{VulkanSwapChain::MAX_FRAMES_IN_FLIGHT};

        std::unique_ptr<VulkanDescriptorPool> m_globalPool{};

        std::unique_ptr<SimpleRendererSystem> m_simpleRendererSystem;
        std::unique_ptr<PointLightSystem> m_pointLightSystem;
        std::unique_ptr<CameraSystem> m_cameraSystem;
    };
}
