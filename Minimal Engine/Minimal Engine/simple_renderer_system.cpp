#include "simple_renderer_system.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace minimal
{
    struct simple_push_constant_data
    {
        glm::mat4 transform{1.0f};
        alignas(16) glm::vec3 color;
    };

    simple_renderer_system::simple_renderer_system(device& device, VkRenderPass render_pass) : device_{device}
    {
        create_pipeline_layout();
        create_pipeline(render_pass);
    }

    simple_renderer_system::~simple_renderer_system()
    {
        vkDestroyPipelineLayout(device_.get_device(), pipeline_layout_, nullptr);
    }

    void simple_renderer_system::create_pipeline_layout()
    {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(simple_push_constant_data);

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void simple_renderer_system::create_pipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

        pipeline_config_info pipeline_config{};
        pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<pipeline>(device_, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipeline_config);
    }

    void simple_renderer_system::render_game_objects(VkCommandBuffer command_buffer, std::vector<game_object>& game_objects, const camera& camera)
    {
        pipeline_->bind(command_buffer);

        auto projection_view = camera.get_projection() * camera.get_view();

        for (auto& obj : game_objects)
        {
            obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.001f, glm::two_pi<float>());
            obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.0005f, glm::two_pi<float>());

            simple_push_constant_data push{};
            push.color = obj.color;
            push.transform = projection_view * obj.transform.mat4();

            vkCmdPushConstants(command_buffer,
                               pipeline_layout_,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(push),
                               &push);
            obj.model->bind(command_buffer);
            obj.model->draw(command_buffer);
        }
    }
}
