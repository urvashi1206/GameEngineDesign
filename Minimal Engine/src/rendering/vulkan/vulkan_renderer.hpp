#pragma once

#include <assert.h>
#include <memory>

#include "vulkan_device.hpp"
#include "../../window.hpp"
#include "vulkan_swap_chain.hpp"

namespace minimal {
    class vulkan_renderer {
    public:
        vulkan_renderer(window &window, vulkan_device &device);

        ~vulkan_renderer();

        vulkan_renderer(const vulkan_renderer &) = delete;

        vulkan_renderer &operator=(const vulkan_renderer &) = delete;

        VkRenderPass get_swap_chain_render_pass() const { return swap_chain_->getRenderPass(); }
        float get_aspect_ratio() const { return swap_chain_->extentAspectRatio(); }
        bool is_frame_in_progress() const { return is_frame_started_; }

        VkCommandBuffer get_current_command_buffer() const {
            assert(is_frame_started_ && "Cannot get command buffer while frame is not in progress");
            return command_buffers_[current_frame_index_];
        }

        int get_frame_index() const {
            assert(is_frame_started_ && "Cannot get frame index while frame is not in progress");
            return current_frame_index_;
        }

        VkCommandBuffer begin_frame();

        void end_frame();

        void being_swap_chain_render_pass(VkCommandBuffer command_buffer);

        void end_swap_chain_render_pass(VkCommandBuffer command_buffer);

    private:
        void create_command_buffers();

        void free_command_buffers();

        void recreate_swap_chain();

        window &window_;
        vulkan_device &device_;
        std::unique_ptr<vulkan_swap_chain> swap_chain_;
        std::vector<VkCommandBuffer> command_buffers_;

        uint32_t current_image_index_;
        int current_frame_index_{0};
        bool is_frame_started_{false};
    };
}
