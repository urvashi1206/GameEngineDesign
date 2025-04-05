//
// Created by rohit on 4/4/2025.
//
#pragma once

#include "FrameInfo.hpp"

namespace Minimal {
    class CameraSystem {
    public:
        CameraSystem(ECSCoordinator &ecs);

        ~CameraSystem() = default;

        void setOrthographicProjection(Entity cameraEntity, float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(Entity cameraEntity, float fovY, float aspect, float near, float far);

        void update(FrameInfo &frameInfo, GlobalUBO &ubo);

    private:
        ECSCoordinator &m_ecs;

        glm::mat4 getProjectionMatrix(CameraComponent &camera);
    };
}
