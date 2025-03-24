#include "keyboard_movement_controller.hpp"

namespace minimal
{
    void keyboard_movement_controller::move_in_plane_xz(GLFWwindow* window, float dt, game_object& game_object)
    {
        glm::vec3 rotate{0};
        if (glfwGetKey(window, keys.look_right) == GLFW_PRESS)
            rotate.y += 1.0f;
        if (glfwGetKey(window, keys.look_left) == GLFW_PRESS)
            rotate.y -= 1.0f;
        if (glfwGetKey(window, keys.look_up) == GLFW_PRESS)
            rotate.x += 1.0f;
        if (glfwGetKey(window, keys.look_down) == GLFW_PRESS)
            rotate.x -= 1.0f;

        if (dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
            game_object.transform.rotation += look_speed * dt * normalize(rotate);

        // limit pitch values between +/- 85ish degrees
        game_object.transform.rotation.x = glm::clamp(game_object.transform.rotation.x, -1.5f, 1.5f);
        game_object.transform.rotation.y = glm::mod(game_object.transform.rotation.y, glm::two_pi<float>());

        float yaw = game_object.transform.rotation.y;
        const glm::vec3 forward_dir{sin(yaw), 0.0f, cos(yaw)};
        const glm::vec3 right_dir{forward_dir.z, 0.0f, -forward_dir.x};
        constexpr glm::vec3 up_dir{0.0f, -1.0f, 0.0f};

        glm::vec3 move_dir{0.0f};
        if (glfwGetKey(window, keys.move_forward) == GLFW_PRESS)
            move_dir += forward_dir;
        if (glfwGetKey(window, keys.move_back) == GLFW_PRESS)
            move_dir -= forward_dir;
        if (glfwGetKey(window, keys.move_right) == GLFW_PRESS)
            move_dir += right_dir;
        if (glfwGetKey(window, keys.move_left) == GLFW_PRESS)
            move_dir -= right_dir;
        if (glfwGetKey(window, keys.move_up) == GLFW_PRESS)
            move_dir += up_dir;
        if (glfwGetKey(window, keys.move_down) == GLFW_PRESS)
            move_dir -= up_dir;

        if (dot(move_dir, move_dir) > std::numeric_limits<float>::epsilon())
            game_object.transform.translation += move_speed * dt * normalize(move_dir);
    }
}
