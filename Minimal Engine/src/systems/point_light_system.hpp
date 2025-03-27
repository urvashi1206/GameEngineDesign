#pragma once

#include <memory>

#include "camera.hpp"
#include "device.hpp"
#include "frame_info.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"

namespace minimal
{
    class point_light_system
    {
    public:
        point_light_system(device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~point_light_system();

        point_light_system(const point_light_system&) = delete;
        point_light_system& operator=(const point_light_system&) = delete;

        void update(frame_info& frame_info, global_ubo& ubo);
        void render(frame_info& frame_info);

    private:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
        void create_pipeline(VkRenderPass render_pass);

        device& device_;

        std::unique_ptr<pipeline> pipeline_;
        VkPipelineLayout pipeline_layout_;
    };
}
