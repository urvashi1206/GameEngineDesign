#pragma once

#include "Model.hpp"

namespace Minimal {
    struct Transform3D {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
    };

    struct MeshRendererComponent {
        std::shared_ptr<Model> model;
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
        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(float fovy, float aspect, float near, float far);

        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = {0.0f, -1.0f, 0.0f});

        void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.0f, -1.0f, 0.0f});

        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4 &getProjection() const { return m_projectionMatrix; }
        const glm::mat4 &getView() const { return m_viewMatrix; }
        const glm::mat4 &getInverseView() const { return m_inverseViewMatrix; }
        const glm::vec3 &getPosition() const { return glm::vec3(m_inverseViewMatrix[3]); }

    private:
        glm::mat4 m_projectionMatrix{1.0f};
        glm::mat4 m_viewMatrix{1.0f};
        glm::mat4 m_inverseViewMatrix{1.0f};
    };
}
