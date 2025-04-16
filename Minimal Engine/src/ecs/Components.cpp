//
// Created by rohit on 4/3/2025.
//
#include "Components.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace Minimal {
    void TransformComponent::rotate(glm::quat quaternion) {
        /*if (quaternion == glm::quat)
            return;*/
        
        rotation = quaternion * rotation;
    }

    glm::mat4 TransformComponent::mat4() {
        glm::mat4 translation = translate(glm::mat4(1.0f), position);
        glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::roll(rotation), glm::vec3(0, 0, 1));
        glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::pitch(rotation), glm::vec3(1, 0, 0));
        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::yaw(rotation), glm::vec3(0, 1, 0));
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

        // Note: The order of rotations must match your intended convention.
        glm::mat4 rotationMat = glm::mat3(rotation);

        return translation * rotationMat * scaling;
    }

    glm::mat3 TransformComponent::normalMatrix() {
        const float c3 = glm::cos(glm::roll(rotation));
        const float s3 = glm::sin(glm::roll(rotation));
        const float c2 = glm::cos(glm::pitch(rotation));
        const float s2 = glm::sin(glm::pitch(rotation));
        const float c1 = glm::cos(glm::yaw(rotation));
        const float s1 = glm::sin(glm::yaw(rotation));
        const glm::vec3 inverseScale = 1.0f / scale;
        return glm::mat3{
            {
                inverseScale.x * (c1 * c3 + s1 * s2 * s3),
                inverseScale.x * (c2 * s3),
                inverseScale.x * (c1 * s2 * s3 - c3 * s1)
            },
            {
                inverseScale.y * (c3 * s1 * s2 - c1 * s3),
                inverseScale.y * (c2 * c3),
                inverseScale.y * (c1 * c3 * s2 + s1 * s3)
            },
            {
                inverseScale.z * (c2 * s1),
                inverseScale.z * (-s2),
                inverseScale.z * (c1 * c2)
            }
        };
    }
    glm::vec3 TransformComponent::right() const {
        return rotation * glm::vec3(1, 0, 0);
    }
    glm::vec3 TransformComponent::up() const {
        return rotation * glm::vec3(0, 1, 0);
    }
    glm::vec3 TransformComponent::forward() const {
        return rotation * glm::vec3(0, 0, 1);
    }

    std::string TransformComponent::toString() {
        return "TransformComponent: " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + "\n" +
               std::to_string(glm::pitch(rotation)) + ", " + std::to_string(glm::yaw(rotation)) + ", " + std::to_string(glm::roll(rotation)) + "\n" +
               std::to_string(scale.x) + ", " + std::to_string(scale.y) + ", " + std::to_string(scale.z);
    }
}
