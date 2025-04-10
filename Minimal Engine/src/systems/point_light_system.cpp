#include "point_light_system.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace minimal
{
    point_light_system::point_light_system(device& device, VkRenderPass render_pass,
                                           VkDescriptorSetLayout global_set_layout) : device_{device}
    {
        create_pipeline_layout(global_set_layout);
        create_pipeline(render_pass);
    }

    point_light_system::~point_light_system()
    {
        vkDestroyPipelineLayout(device_.get_device(), pipeline_layout_, nullptr);
    }

    void point_light_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
    {
        //        VkPushConstantRange push_constant_range{};
        //        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        //        push_constant_range.offset = 0;
        //        push_constant_range.size = sizeof(simple_push_constant_data);

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        //        pipeline_layout_info.pushConstantRangeCount = 1;
        //        pipeline_layout_info.pPushConstantRanges = &push_constant_range;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout_) !=
            VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void point_light_system::create_pipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

        pipeline_config_info pipeline_config{};
        pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.binding_descriptions.clear();
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<pipeline>(device_,
                                               "shaders\\point_light.vert.spv",
                                               "shaders\\point_light.frag.spv",
                                               pipeline_config);
    }

    void point_light_system::render(frame_info& frame_info)
    {
        pipeline_->bind(frame_info.command_buffer);

        auto projection_view = frame_info.camera.get_projection() * frame_info.camera.get_view();

        vkCmdBindDescriptorSets(
            frame_info.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout_,
            0,
            1,
            &frame_info.global_descriptor_set,
            0,
            nullptr
        );

        vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
    }
}
