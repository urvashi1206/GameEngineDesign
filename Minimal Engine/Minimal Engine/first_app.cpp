#include "first_app.hpp"

#include "simple_renderer_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <algorithm>
#include <glm/glm.hpp>

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
        while (!window_.should_close())
        {
            glfwPollEvents();

            if (auto command_buffer = renderer_.begin_frame())
            {
                renderer_.being_swap_chain_render_pass(command_buffer);
                simple_renderer_system.render_game_objects(command_buffer, game_objects_);
                renderer_.end_swap_chain_render_pass(command_buffer);
                renderer_.end_frame();
            }
        }

        vkDeviceWaitIdle(device_.get_device());
    } // temporary helper function, creates a 1x1x1 cube centered at offset
    std::unique_ptr<model> createCubeModel(device& device, glm::vec3 offset)
    {
        std::vector<model::vertex> vertices{

            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

        };
        for (auto& v : vertices)
        {
            v.position += offset;
        }
        return std::make_unique<model>(device, vertices);
    }

    void first_app::load_game_objects()
    {
        std::shared_ptr<model> model = createCubeModel(device_, {0.0f, 0.0f, 0.0f});

        auto cube = game_object::create();
        cube.model = model;
        cube.transform.translation= {0.0f, 0.0f, 0.5f};
        cube.transform.scale = {0.5f, 0.5f, 0.5f};
        game_objects_.push_back(std::move(cube));
    }
}
