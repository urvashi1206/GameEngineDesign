#pragma once

#include "Mesh.hpp"

#include <glm/gtc/quaternion.hpp>

namespace Minimal {
    struct TransformComponent {
        glm::vec3 position{0.0f};
        glm::quat rotation{0, 0, 0, 1};
        glm::vec3 scale{1.0f};

        void rotate(glm::quat quaternion);

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix // Brendan Galea
        glm::mat4 mat4();

        glm::mat3 normalMatrix();

        glm::vec3 right() const;

        glm::vec3 up() const;

        glm::vec3 forward() const;
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
        bool isStatic;

        float mass = 1;
        float bounciness = 0;
        float staticFriction = 0.5f;
        float dynamicFriction = 0.3f;

        glm::vec3 gravity;

        glm::vec3 velocity;
        glm::vec3 angularVelocity;

        glm::vec3 netForce{0, 0, 0};
        glm::vec3 netTorque{0, 0, 0};
    };

    struct RenderComponent {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        bool wireframe = false;
        float size = 1.0f;
    };

    enum class EColliderType {
        None,
        Box,
        Sphere
    };

    struct ColliderComponent {
        EColliderType colliderType;

        glm::vec3 center;
        glm::vec3 halfSize;
        float radius;
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
        glm::vec3 getPosition() { return glm::vec3(inverseViewMatrix[3]); }

        bool isMain;
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};
    };
}
