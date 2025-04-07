//
// Created by rohit on 4/4/2025.
//
#pragma once

#include "FrameInfo.hpp"

namespace Minimal {
    class CameraSystem {
    public:
        CameraSystem(ECSCoordinator &ecs);

        ~CameraSystem();

        CameraSystem(const CameraSystem &) = delete;

        CameraSystem &operator=(const CameraSystem &) = delete;

        void setOrthographicProjection(Entity cameraEntity, float left, float right, float top, float bottom, float near, float far);

        void setOrthographicProjection(CameraComponent &camera, float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(Entity cameraEntity, float fovY, float aspect, float near, float far);

        void setPerspectiveProjection(CameraComponent &camera, float fovY, float aspect, float near, float far);

        void setViewDirection(Entity cameraEntity, glm::vec3 direction, glm::vec3 up = {0.0f, -1.0f, 0.0f});

        void setViewTarget(Entity cameraEntity, glm::vec3 target, glm::vec3 up = {0.0f, -1.0f, 0.0f});

        void setViewYXZ(Entity cameraEntity);

        void setViewYXZ(CameraComponent &camera, TransformComponent &transform);

        CameraComponent &getMainCamera();

        void update(float aspect);

    private:
        ECSCoordinator &m_ecs;

        bool hasCamera(Entity cameraEntity);

        CameraComponent &getCamera(Entity cameraEntity);
    };
}
