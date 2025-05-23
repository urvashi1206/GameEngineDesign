#include "KeyboardMovementController.hpp"

#include <glm/gtc/constants.hpp>


namespace Minimal {
    void KeyboardMovementController::moveInPlaneXZ(GLFWwindow *window, float dt, TransformComponent &transform) {
        glm::vec3 rotate{0};
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
            rotate.y += 1.0f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
            rotate.y -= 1.0f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
            rotate.x += 1.0f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
            rotate.x -= 1.0f;

        if (dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
            transform.rotate(glm::quat(lookSpeed * dt * normalize(rotate)));

        // limit pitch values between +/- 85ish degrees
        // TO-DO: UPDATE TO QUATERNIONS
        transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
        transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

        float yaw = glm::yaw(transform.rotation);
        const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
        constexpr glm::vec3 upDir{0.0f, -1.0f, 0.0f};

        glm::vec3 moveDir{0.0f};
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
            moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBack) == GLFW_PRESS)
            moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
            moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
            moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
            moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
            moveDir -= upDir;

        if (dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
            transform.position += moveSpeed * dt * normalize(moveDir);
    }
}
