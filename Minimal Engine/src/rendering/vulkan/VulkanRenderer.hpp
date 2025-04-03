#pragma once

#include <assert.h>
#include <memory>

#include "VulkanDevice.hpp"
#include "../../Window.hpp"
#include "VulkanSwapChain.hpp"

namespace Minimal {
    class VulkanRenderer {
    public:
        VulkanRenderer(Window &window, VulkanDevice &device);

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

        VkCommandBuffer beginFrame();

        void endFrame();

        void beingSwapChainRenderPass(VkCommandBuffer commandBuffer);

        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();

        void freeCommandBuffers();

        void recreateSwapChain();

        Window &m_window;
        VulkanDevice &m_device;
        std::unique_ptr<VulkanSwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex;
        int m_currentFrameIndex{0};
        bool m_isFrameStarted{false};
    };
}
