//
// Created by rohit on 4/3/2025.
//
#include "Components.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace Minimal {
    glm::mat4 TransformComponent::mat4() {
        glm::mat4 translation = translate(glm::mat4(1.0f), position);
        glm::mat4 rotationZ = rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0, 0, 1));
        glm::mat4 rotationX = rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
        glm::mat4 rotationY = rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

        // Note: The order of rotations must match your intended convention.
        glm::mat4 rotationMat = rotationY * rotationX * rotationZ;

        return translation * rotationMat * scaling;
    }

    glm::mat3 TransformComponent::normalMatrix() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
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

    std::string TransformComponent::toString() {
        return "TransformComponent: " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + "\n" +
               std::to_string(rotation.x) + ", " + std::to_string(rotation.y) + ", " + std::to_string(rotation.z) + "\n" +
               std::to_string(scale.x) + ", " + std::to_string(scale.y) + ", " + std::to_string(scale.z);
    }
}
