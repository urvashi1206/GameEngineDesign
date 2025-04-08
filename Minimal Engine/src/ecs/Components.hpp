#pragma once

#include "Mesh.hpp"

namespace Minimal {
    struct TransformComponent {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix // Brendan Galea
        glm::mat4 mat4();

        glm::mat3 normalMatrix();

        std::string toString();
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
        glm::vec3 color{};
    };

    struct MeshRendererComponent {
        std::shared_ptr<Mesh> mesh;
        // will have a material here at a later point.
    };

    struct RigidbodyComponent {
        glm::vec3 velocity;
    };

    struct RenderComponent {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        bool wireframe = false;
        float size = 1.0f;
    };

    struct ColliderComponent {
        float width;
        float height;
        float depth;
    };

    struct HealthComponent {
        int current;
        int max;
    };

    struct InputComponent {
        bool moveLeft;
        bool moveRight;
        bool moveUp;
        bool moveDown;
    };

    struct CameraComponent {

        const glm::vec3 &getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

        bool isMain;
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};
    };
}
