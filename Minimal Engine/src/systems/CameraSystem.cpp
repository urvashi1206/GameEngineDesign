//
// Created by rohit on 4/4/2025.
//

#include "CameraSystem.hpp"

namespace Minimal {
    CameraSystem::CameraSystem(ECSCoordinator &ecs): RenderSubsystem(ecs) {}

    void CameraSystem::processOrthographicProjection(glm::mat4 &projectionMatrix, float left, float right, float top, float bottom, float near, float far) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.0f / (right - left);
        projectionMatrix[1][1] = 2.0f / (bottom - top);
        projectionMatrix[2][2] = 1.0f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }

    void CameraSystem::processPerspectiveProjection(glm::mat4 &projectionMatrix, float fovY, float aspect, float near, float far) {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f && "aspect cannot be zero");
        const float tanHalfFovy = tan(fovY / 2.f);

        projectionMatrix = glm::mat4{0.0f};
        projectionMatrix[0][0] = 1.0f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.0f / (tanHalfFovy);
        projectionMatrix[2][2] = far / (far - near);
        projectionMatrix[2][3] = 1.0f;
        projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    void CameraSystem::processViewYXZ(glm::mat4 &viewMatrix, glm::mat4 &inverseViewMatrix, const TransformComponent &transform) {
        const float c3 = glm::cos(glm::roll(transform.rotation));
        const float s3 = glm::sin(glm::roll(transform.rotation));
        const float c2 = glm::cos(glm::pitch(transform.rotation));
        const float s2 = glm::sin(glm::pitch(transform.rotation));
        const float c1 = glm::cos(glm::yaw(transform.rotation));
        const float s1 = glm::sin(glm::yaw(transform.rotation));
        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};

        viewMatrix = glm::mat4{1.0f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -dot(u, transform.position);
        viewMatrix[3][1] = -dot(v, transform.position);
        viewMatrix[3][2] = -dot(w, transform.position);

        inverseViewMatrix = glm::mat4{1.0f};
        inverseViewMatrix[0][0] = u.x;
        inverseViewMatrix[0][1] = u.y;
        inverseViewMatrix[0][2] = u.z;
        inverseViewMatrix[1][0] = v.x;
        inverseViewMatrix[1][1] = v.y;
        inverseViewMatrix[1][2] = v.z;
        inverseViewMatrix[2][0] = w.x;
        inverseViewMatrix[2][1] = w.y;
        inverseViewMatrix[2][2] = w.z;
        inverseViewMatrix[3][0] = transform.position.x;
        inverseViewMatrix[3][1] = transform.position.y;
        inverseViewMatrix[3][2] = transform.position.z;
    }

    void CameraSystem::update(FrameInfo &frameInfo) {
        CameraComponent *mainCamera{nullptr};
        Entity mainCameraEntity{MAX_ENTITIES + 1};
        CameraComponent *fallbackCamera{nullptr};
        Entity fallbackCameraEntity{MAX_ENTITIES + 1};

        for (Entity entity = 0; entity < m_ecs.getEntityCount(); entity++) {
            if (!hasCamera(entity))
                continue;

            auto &camera = getCamera(entity);
            auto &cameraTransform = m_ecs.getComponent<TransformComponent>(entity);

            processViewYXZ(camera.viewMatrix, camera.inverseViewMatrix, cameraTransform);
            // processOrthographicProjection(camera.projectionMatrix, -frameInfo.aspect, frameInfo.aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            processPerspectiveProjection(camera.projectionMatrix, glm::radians(50.0f), frameInfo.aspect, 0.1f, 100.0f);

            if (mainCamera != nullptr)
                continue;

            if (camera.isMain) {
                mainCamera = &camera;
                mainCameraEntity = entity;
            }

            if (fallbackCamera == nullptr) {
                fallbackCamera = &camera;
                fallbackCameraEntity = entity;
            }
        }

        if (mainCamera == nullptr) {
            mainCamera = fallbackCamera;
            mainCameraEntity = fallbackCameraEntity;
        }

        assert(hasCamera(mainCameraEntity) && "No cameras available");

        auto &cameraTransform = m_ecs.getComponent<TransformComponent>(mainCameraEntity);
        auto &camera = getCamera(mainCameraEntity);

        frameInfo.camera = &camera;
        processViewYXZ(frameInfo.ubo.view, frameInfo.ubo.inverseView, cameraTransform);
        frameInfo.ubo.projection = mainCamera->projectionMatrix;
        frameInfo.ubo.view = mainCamera->viewMatrix;
        frameInfo.ubo.inverseView = mainCamera->inverseViewMatrix;
    }

    void CameraSystem::render(FrameInfo &frameInfo) {}

    bool CameraSystem::hasCamera(Entity cameraEntity) {
        return m_ecs.hasComponent<CameraComponent>(cameraEntity);
    }

    CameraComponent &CameraSystem::getCamera(Entity cameraEntity) {
        assert(hasCamera(cameraEntity) && "Camera does not exist");
        return m_ecs.getComponent<CameraComponent>(cameraEntity);
    }
}
