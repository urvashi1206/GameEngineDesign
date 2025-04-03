#pragma once

#include <memory>

#include "camera.hpp"
#include "../rendering/vulkan/vulkan_device.hpp"
#include "frame_info.hpp"
#include "game_object.hpp"
#include "rendering/vulkan/vulkan_pipeline.hpp"

namespace minimal
{
    class simple_renderer_system
    {
    public:
        simple_renderer_system(vulkan_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~simple_renderer_system();

        simple_renderer_system(const simple_renderer_system&) = delete;
        simple_renderer_system& operator=(const simple_renderer_system&) = delete;

        void render_game_objects(frame_info& frame_info);

    private:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
        void create_pipeline(VkRenderPass render_pass);

        vulkan_device& device_;

        std::unique_ptr<vulkan_pipeline> pipeline_;
        VkPipelineLayout pipeline_layout_;
    };
}
