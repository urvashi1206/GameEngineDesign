#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "simple_renderer_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <chrono>

namespace minimal
{
    struct global_ubo
    {
        glm::mat4 projection_view{1.0f};
        glm::vec3 light_direction = normalize(glm::vec3(1.0f, -3.0f, -2.0f));
    };

    first_app::first_app()
    {
        load_game_objects();
    }

    first_app::~first_app() {}

    void first_app::run()
    {
        buffer global_ubo_buffer{
            device_,
            sizeof(global_ubo),
            swap_chain::MAX_FRAMES_IN_FLIGHT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            device_.properties.limits.minUniformBufferOffsetAlignment
        };

        global_ubo_buffer.map();

        simple_renderer_system simple_renderer_system{device_, renderer_.get_swap_chain_render_pass()};
        camera camera{};
        // camera.set_view_direction(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        camera.set_view_target(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewer_object = game_object::create();
        keyboard_movement_controller camera_controller{};

        auto current_time = std::chrono::high_resolution_clock::now();

        while (!window_.should_close())
        {
            glfwPollEvents();

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;

            camera_controller.move_in_plane_xz(window_.get_glfw_window(), frame_time, viewer_object);
            camera.set_view_YXZ(viewer_object.transform.translation, viewer_object.transform.rotation);

            float aspect = renderer_.get_aspect_ratio();
            // camera.set_othrographic_projection(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            camera.set_perspective_projection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            if (auto command_buffer = renderer_.begin_frame())
            {
                int frame_index = renderer_.get_frame_index();

                frame_info frame_info{
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera
                };

                // update
                global_ubo ubo{};
                ubo.projection_view = camera.get_projection() * camera.get_view();
                global_ubo_buffer.writeToBuffer(&ubo, frame_index);
                global_ubo_buffer.flushIndex(frame_index);

                // render
                renderer_.being_swap_chain_render_pass(command_buffer);
                simple_renderer_system.render_game_objects(frame_info, game_objects_);
                renderer_.end_swap_chain_render_pass(command_buffer);
                renderer_.end_frame();
            }
        }

        vkDeviceWaitIdle(device_.get_device());
    }

    void first_app::load_game_objects()
    {
        std::shared_ptr<model> model = model::create_model_from_file(
            device_, "C:\\Users\\rohit\\Study\\Sem 4\\689\\Game Engine Project\\Minimal Engine\\Minimal Engine\\models\\flat_vase.obj");

        auto flat_vase = game_object::create();
        flat_vase.model = model;
        flat_vase.transform.translation = {-0.5f, 0.5f, 2.5f};
        flat_vase.transform.scale = {3.0f, 1.5f, 3.0f};
        game_objects_.push_back(std::move(flat_vase));

        model = model::create_model_from_file(
            device_, "C:\\Users\\rohit\\Study\\Sem 4\\689\\Game Engine Project\\Minimal Engine\\Minimal Engine\\models\\smooth_vase.obj");

        auto smooth_vase = game_object::create();
        smooth_vase.model = model;
        smooth_vase.transform.translation = {0.5f, 0.5f, 2.5f};
        smooth_vase.transform.scale = {3.0f, 1.5f, 3.0f};
        game_objects_.push_back(std::move(smooth_vase));
    }
}
