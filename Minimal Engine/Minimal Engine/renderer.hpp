#pragma once

#include <assert.h>
#include <memory>

#include "device.hpp"
#include "window.hpp"
#include "swap_chain.hpp"

namespace minimal
{
    class renderer
    {
    public:
        renderer(window& window, device& device);
        ~renderer();

        renderer(const renderer&) = delete;
        renderer& operator=(const renderer&) = delete;

        VkRenderPass get_swap_chain_render_pass()const{return swap_chain_->getRenderPass();}
        bool is_frame_in_progress() const { return is_frame_started_; }

        VkCommandBuffer get_current_command_buffer() const
        {
            assert(is_frame_started_ && "Cannot get command buffer while frame is not in progress");
            return command_buffers_[current_frame_index_];
        }

        int get_current_frame_index() const
        {
            assert(is_frame_started_);
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

        window& window_;
        device& device_;
        std::unique_ptr<swap_chain> swap_chain_;
        std::vector<VkCommandBuffer> command_buffers_;

        uint32_t current_image_index_;
        int current_frame_index_;
        bool is_frame_started_ {false};
    };
}
