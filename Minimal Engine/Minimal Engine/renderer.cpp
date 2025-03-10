#include "renderer.hpp"

#include <array>
#include <iostream>
#include <stdexcept>


namespace minimal
{
    renderer::renderer(window& window, device& device) : window_{window}, device_{device}
    {
        recreate_swap_chain();
        create_command_buffers();
    }

    renderer::~renderer()
    {
        free_command_buffers();
    }

    VkCommandBuffer renderer::begin_frame()
    {
        assert(!is_frame_in_progress() && "Cannot call begin_frame while frame is in progress");

        auto result = swap_chain_->acquireNextImage(&current_image_index_);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swap_chain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");

        is_frame_started_ = true;

        auto command_buffer = get_current_command_buffer();

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        return command_buffer;
    }

    void renderer::end_frame()
    {
        assert(is_frame_started_ && "Cannot call end_frame while frame is not in progress");
        auto command_buffer = get_current_command_buffer();
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffers!");

        auto result = swap_chain_->submitCommandBuffers(&command_buffer, &current_image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.was_window_resized())
        {
            window_.reset_window_resized_flag();
            recreate_swap_chain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit swap chain command buffers!");
        }

        is_frame_started_ = false;
        current_frame_index_ = (current_frame_index_ + 1) % swap_chain::MAX_FRAMES_IN_FLIGHT;
    }

    void renderer::being_swap_chain_render_pass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ && "Cannot begin render pass when frame is not in progress");
        assert(command_buffer == get_current_command_buffer() && "can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = swap_chain_->getRenderPass();
        render_pass_info.framebuffer = swap_chain_->getFrameBuffer(current_image_index_);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swap_chain_->getSwapChainExtent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swap_chain_->width());
        viewport.height = static_cast<float>(swap_chain_->height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swap_chain_->getSwapChainExtent()};
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void renderer::end_swap_chain_render_pass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ && "Cannot end render pass when frame is not in progress");
        assert(command_buffer == get_current_command_buffer() && "can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(command_buffer);
    }

    void renderer::create_command_buffers()
    {
        command_buffers_.resize(swap_chain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = device_.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(device_.get_device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");
    }

    void renderer::free_command_buffers()
    {
        vkFreeCommandBuffers(device_.get_device(), device_.getCommandPool(), static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
        command_buffers_.clear();
    }

    void renderer::recreate_swap_chain()
    {
        auto extent = window_.get_extent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = window_.get_extent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device_.get_device());

        if (swap_chain_ == nullptr)
        {
            swap_chain_ = std::make_unique<swap_chain>(device_, extent);
        }
        else
        {
            std::shared_ptr<swap_chain> old_swap_chain = std::move(swap_chain_);
            swap_chain_ = std::make_unique<swap_chain>(device_, extent, old_swap_chain);

            if (!old_swap_chain->compareSwapFormats(*swap_chain_.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }
}
