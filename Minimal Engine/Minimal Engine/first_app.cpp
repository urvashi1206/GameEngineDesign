#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
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
    first_app::first_app()
    {
        load_game_objects();
    }

    first_app::~first_app()
    {
    }

    void first_app::run()
    {
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
                renderer_.being_swap_chain_render_pass(command_buffer);
                simple_renderer_system.render_game_objects(command_buffer, game_objects_, camera);
                renderer_.end_swap_chain_render_pass(command_buffer);
                renderer_.end_frame();
            }
        }

        vkDeviceWaitIdle(device_.get_device());
    }

    void first_app::load_game_objects()
    {
        std::shared_ptr<model> model = model::create_model_from_file(
            device_, "C:\\Users\\rohit\\Study\\Sem 4\\689\\Game Engine Project\\Minimal Engine\\Minimal Engine\\models\\smooth_vase.obj");

        auto game_obj = game_object::create();
        game_obj.model = model;
        game_obj.transform.translation = {0.0f, 0.0f, 2.5f};
        game_obj.transform.scale = glm::vec3(3.0f);
        game_objects_.push_back(std::move(game_obj));
    }
}
