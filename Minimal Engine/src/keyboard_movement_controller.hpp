#pragma once

#include "game_object.hpp"
#include "window.hpp"

namespace minimal
{
    class keyboard_movement_controller
    {
    public:
        struct key_mappings
        {
            int move_left = GLFW_KEY_A;
            int move_right = GLFW_KEY_D;
            int move_forward = GLFW_KEY_W;
            int move_back = GLFW_KEY_S;
            int move_up = GLFW_KEY_SPACE;
            int move_down = GLFW_KEY_LEFT_SHIFT;
            int look_left = GLFW_KEY_LEFT;
            int look_right = GLFW_KEY_RIGHT;
            int look_up = GLFW_KEY_UP;
            int look_down = GLFW_KEY_DOWN;
        };

        void move_in_plane_xz(GLFWwindow* window, float dt, game_object& game_object);
        
        key_mappings keys{};
        float move_speed{3.0f};
        float look_speed{3.0f};
    };
}
