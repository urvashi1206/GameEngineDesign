//
// Created by rohit on 4/4/2025.
//
#pragma once

#include "../rendering/RenderSubsystem.hpp"

namespace Minimal {
    class CameraSystem final : public RenderSubsystem {
    public:
        CameraSystem(ECSCoordinator &ecs);

        CameraSystem(const CameraSystem &) = delete;

        CameraSystem &operator=(const CameraSystem &) = delete;

        void processOrthographicProjection(glm::mat4 &projectionMatrix, float left, float right, float top, float bottom, float near, float far);

        void processPerspectiveProjection(glm::mat4 &projectionMatrix, float fovY, float aspect, float near, float far);

        void processViewYXZ(glm::mat4 &viewMatrix, glm::mat4 &inverseViewMatrix, const TransformComponent &transform);

        void update(FrameInfo &frameInfo) override;

        void render(FrameInfo &frameInfo) override;

    private:
        bool hasCamera(Entity cameraEntity);

        CameraComponent &getCamera(Entity cameraEntity);
    };
}
