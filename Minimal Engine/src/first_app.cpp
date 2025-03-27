#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "systems/simple_renderer_system.hpp"
#include "systems/point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>

namespace minimal {
    first_app::first_app() {
        global_pool_ = descriptor_pool::builder(device_)
                .setMaxSets(swap_chain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                .build();

        load_game_objects();
    }

    first_app::~first_app() {
    }

    void first_app::run() {
        std::vector<std::unique_ptr<buffer> > ubo_buffers(swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); i++) {
            ubo_buffers[i] = std::make_unique<buffer>(
                device_,
                sizeof(global_ubo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            ubo_buffers[i]->map();
        }

        auto global_set_layout = descriptor_set_layout::builder(device_)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        std::vector<VkDescriptorSet> global_descriptor_sets(swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < swap_chain::MAX_FRAMES_IN_FLIGHT; i++) {
            auto buffer_info = ubo_buffers[i]->descriptorInfo();
            descriptor_writer(*global_set_layout, *global_pool_)
                    .writeBuffer(0, &buffer_info)
                    .build(global_descriptor_sets[i]);
        }

        simple_renderer_system simple_renderer_system{
            device_,
            renderer_.get_swap_chain_render_pass(),
            global_set_layout->getDescriptorSetLayout()
        };

        point_light_system point_light_system{
            device_,
            renderer_.get_swap_chain_render_pass(),
            global_set_layout->getDescriptorSetLayout()
        };

        camera camera{};
        // camera.set_view_direction(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        camera.set_view_target(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewer_object = game_object::create();
        viewer_object.transform.translation.z = -2.5f;

        keyboard_movement_controller camera_controller{};

        auto current_time = std::chrono::high_resolution_clock::now();

        while (!window_.should_close()) {
            glfwPollEvents();

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).
                    count();
            current_time = new_time;

            camera_controller.move_in_plane_xz(window_.get_glfw_window(), frame_time, viewer_object);
            camera.set_view_YXZ(viewer_object.transform.translation, viewer_object.transform.rotation);

            float aspect = renderer_.get_aspect_ratio();
            // camera.set_othrographic_projection(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            camera.set_perspective_projection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            if (auto command_buffer = renderer_.begin_frame()) {
                int frame_index = renderer_.get_frame_index();

                frame_info frame_info{
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera,
                    global_descriptor_sets[frame_index],
                    game_objects_
                };

                // update
                global_ubo ubo{};
                ubo.projection = camera.get_projection();
                ubo.view = camera.get_view();
                point_light_system.update(frame_info, ubo);
                ubo_buffers[frame_index]->writeToBuffer(&ubo);
                ubo_buffers[frame_index]->flush();

                // render
                renderer_.being_swap_chain_render_pass(command_buffer);

                simple_renderer_system.render_game_objects(frame_info);
                point_light_system.render(frame_info);

                renderer_.end_swap_chain_render_pass(command_buffer);
                renderer_.end_frame();
            }
        }

        vkDeviceWaitIdle(device_.get_device());
    }

    void first_app::load_game_objects() {
        std::shared_ptr<model> model = model::create_model_from_file(device_, "models/flat_vase.obj");

        auto flat_vase = game_object::create();
        flat_vase.model = model;
        flat_vase.transform.translation = {-0.5f, 0.5f, 0.0f};
        flat_vase.transform.scale = {3.0f, 1.5f, 3.0f};
        game_objects_.emplace(flat_vase.get_id(), std::move(flat_vase));

        model = model::create_model_from_file(device_, "models/smooth_vase.obj");


        auto smooth_vase = game_object::create();
        smooth_vase.model = model;
        smooth_vase.transform.translation = {0.5f, 0.5f, 0.0f};
        smooth_vase.transform.scale = {3.0f, 1.5f, 3.0f};
        game_objects_.emplace(smooth_vase.get_id(), std::move(smooth_vase));

        model = model::create_model_from_file(device_, "models/quad.obj");

        auto floor = game_object::create();
        floor.model = model;
        floor.transform.translation = {0.0f, 0.5f, 0.0f};
        floor.transform.scale = {3.0f, 1.0f, 3.0f};
        game_objects_.emplace(floor.get_id(), std::move(floor));

        {
            auto point_light = game_object::make_point_light(0.2f);
            game_objects_.emplace(point_light.get_id(), std::move(point_light));
        }

        // using point_light again invalid...
    }
}
