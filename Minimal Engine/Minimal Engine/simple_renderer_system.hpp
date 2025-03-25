#pragma once

#include <memory>

#include "camera.hpp"
#include "device.hpp"
#include "frame_info.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"

namespace minimal
{
    class simple_renderer_system
    {
    public:
        simple_renderer_system(device& device, VkRenderPass render_pass);
        ~simple_renderer_system();

        simple_renderer_system(const simple_renderer_system&) = delete;
        simple_renderer_system& operator=(const simple_renderer_system&) = delete;

        void render_game_objects(frame_info& frame_info, std::vector<game_object>& game_objects);

    private:
        void create_pipeline_layout();
        void create_pipeline(VkRenderPass render_pass);

        device& device_;

        std::unique_ptr<pipeline> pipeline_;
        VkPipelineLayout pipeline_layout_;
    };
}
