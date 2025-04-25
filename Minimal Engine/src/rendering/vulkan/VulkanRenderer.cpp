#include "VulkanRenderer.hpp"

#include <array>
#include <iostream>
#include <stdexcept>


namespace Minimal {
    VulkanRenderer::VulkanRenderer(Window &window, VulkanDevice &device) : m_window{window}, m_device{device} {
        recreateSwapChain();
        createCommandBuffers();
    }

    VulkanRenderer::~VulkanRenderer() {
        freeCommandBuffers();
    }

    VkCommandBuffer VulkanRenderer::beginFrame() {
        assert(!isFrameInProgress() && "Cannot call begin_frame while frame is in progress");

        auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");

        m_isFrameStarted = true;

        auto command_buffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        return command_buffer;
    }

    void VulkanRenderer::endFrame() {
        assert(m_isFrameStarted && "Cannot call end_frame while frame is not in progress");
        auto command_buffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffers!");

        auto result = m_swapChain->submitCommandBuffers(&command_buffer, &m_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit swap chain command buffers!");
        }

        m_isFrameStarted = false;
        m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanRenderer::beingSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(m_isFrameStarted && "Cannot begin render pass when frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->width());
        viewport.height = static_cast<float>(m_swapChain->height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VulkanRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(m_isFrameStarted && "Cannot end render pass when frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanRenderer::shutdown() {
        vkDeviceWaitIdle(m_device.getDevice());
    }

    void VulkanRenderer::createCommandBuffers() {
        m_commandBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_device.getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");
    }

    void VulkanRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(m_device.getDevice(), m_device.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }

    void VulkanRenderer::recreateSwapChain() {
        auto extent = m_window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_device.getDevice());

        if (m_swapChain == nullptr) {
            m_swapChain = std::make_unique<VulkanSwapChain>(m_device, extent);
        } else {
            std::shared_ptr oldSwapChain = std::move(m_swapChain);
            m_swapChain = std::make_unique<VulkanSwapChain>(m_device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }
}
