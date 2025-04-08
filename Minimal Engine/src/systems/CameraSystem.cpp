//
// Created by rohit on 4/4/2025.
//

#include "CameraSystem.hpp"

namespace Minimal {
    CameraSystem::CameraSystem(ECSCoordinator &ecs): System(ecs) {}

    void CameraSystem::setOrthographicProjection(Entity cameraEntity, float left, float right, float top, float bottom, float near, float far) {
        if (hasCamera(cameraEntity))
            return;

        auto &camera = getCamera(cameraEntity);
        setOrthographicProjection(camera, left, right, top, bottom, near, far);
    }

    void CameraSystem::setOrthographicProjection(CameraComponent &camera, float left, float right, float top, float bottom, float near, float far) {
        camera.projectionMatrix = glm::mat4{1.0f};
        camera.projectionMatrix[0][0] = 2.0f / (right - left);
        camera.projectionMatrix[1][1] = 2.0f / (bottom - top);
        camera.projectionMatrix[2][2] = 1.0f / (far - near);
        camera.projectionMatrix[3][0] = -(right + left) / (right - left);
        camera.projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        camera.projectionMatrix[3][2] = -near / (far - near);
    }

    void CameraSystem::setPerspectiveProjection(Entity cameraEntity, float fovY, float aspect, float near, float far) {
        if (hasCamera(cameraEntity))
            return;

        auto &camera = getCamera(cameraEntity);
        setPerspectiveProjection(camera, fovY, aspect, near, far);
    }

    void CameraSystem::setPerspectiveProjection(CameraComponent &camera, float fovY, float aspect, float near, float far) {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f && "aspect cannot be zero");
        const float tanHalfFovy = tan(fovY / 2.f);

        camera.projectionMatrix = glm::mat4{0.0f};
        camera.projectionMatrix[0][0] = 1.0f / (aspect * tanHalfFovy);
        camera.projectionMatrix[1][1] = 1.0f / (tanHalfFovy);
        camera.projectionMatrix[2][2] = far / (far - near);
        camera.projectionMatrix[2][3] = 1.0f;
        camera.projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    void CameraSystem::setViewDirection(Entity cameraEntity, glm::vec3 direction, glm::vec3 up) {
        if (hasCamera(cameraEntity))
            return;

        auto &camera = getCamera(cameraEntity);
        auto &transform = m_ecs.getComponent<TransformComponent>(cameraEntity);

        const glm::vec3 w{normalize(direction)};
        const glm::vec3 u{normalize(cross(w, up))};
        const glm::vec3 v{cross(w, u)};

        camera.viewMatrix = glm::mat4{1.0f};
        camera.viewMatrix[0][0] = u.x;
        camera.viewMatrix[1][0] = u.y;
        camera.viewMatrix[2][0] = u.z;
        camera.viewMatrix[0][1] = v.x;
        camera.viewMatrix[1][1] = v.y;
        camera.viewMatrix[2][1] = v.z;
        camera.viewMatrix[0][2] = w.x;
        camera.viewMatrix[1][2] = w.y;
        camera.viewMatrix[2][2] = w.z;
        camera.viewMatrix[3][0] = -dot(u, transform.position);
        camera.viewMatrix[3][1] = -dot(v, transform.position);
        camera.viewMatrix[3][2] = -dot(w, transform.position);

        camera.inverseViewMatrix = glm::mat4{1.0f};
        camera.inverseViewMatrix[0][0] = u.x;
        camera.inverseViewMatrix[0][1] = u.y;
        camera.inverseViewMatrix[0][2] = u.z;
        camera.inverseViewMatrix[1][0] = v.x;
        camera.inverseViewMatrix[1][1] = v.y;
        camera.inverseViewMatrix[1][2] = v.z;
        camera.inverseViewMatrix[2][0] = w.x;
        camera.inverseViewMatrix[2][1] = w.y;
        camera.inverseViewMatrix[2][2] = w.z;
        camera.inverseViewMatrix[3][0] = transform.position.x;
        camera.inverseViewMatrix[3][1] = transform.position.y;
        camera.inverseViewMatrix[3][2] = transform.position.z;
    }

    void CameraSystem::setViewTarget(Entity cameraEntity, glm::vec3 target, glm::vec3 up) {
        if (hasCamera(cameraEntity))
            return;

        auto &transform = m_ecs.getComponent<TransformComponent>(cameraEntity);

        setViewDirection(cameraEntity, target - transform.position, up);
    }

    void CameraSystem::setViewYXZ(Entity cameraEntity) {
        if (hasCamera(cameraEntity))
            return;

        auto &camera = getCamera(cameraEntity);
        auto &transform = m_ecs.getComponent<TransformComponent>(cameraEntity);
        setViewYXZ(camera, transform);
    }

    void CameraSystem::setViewYXZ(CameraComponent &camera, TransformComponent &transform) {
        const float c3 = glm::cos(transform.rotation.z);
        const float s3 = glm::sin(transform.rotation.z);
        const float c2 = glm::cos(transform.rotation.x);
        const float s2 = glm::sin(transform.rotation.x);
        const float c1 = glm::cos(transform.rotation.y);
        const float s1 = glm::sin(transform.rotation.y);
        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};

        camera.viewMatrix = glm::mat4{1.0f};
        camera.viewMatrix[0][0] = u.x;
        camera.viewMatrix[1][0] = u.y;
        camera.viewMatrix[2][0] = u.z;
        camera.viewMatrix[0][1] = v.x;
        camera.viewMatrix[1][1] = v.y;
        camera.viewMatrix[2][1] = v.z;
        camera.viewMatrix[0][2] = w.x;
        camera.viewMatrix[1][2] = w.y;
        camera.viewMatrix[2][2] = w.z;
        camera.viewMatrix[3][0] = -dot(u, transform.position);
        camera.viewMatrix[3][1] = -dot(v, transform.position);
        camera.viewMatrix[3][2] = -dot(w, transform.position);

        camera.inverseViewMatrix = glm::mat4{1.0f};
        camera.inverseViewMatrix[0][0] = u.x;
        camera.inverseViewMatrix[0][1] = u.y;
        camera.inverseViewMatrix[0][2] = u.z;
        camera.inverseViewMatrix[1][0] = v.x;
        camera.inverseViewMatrix[1][1] = v.y;
        camera.inverseViewMatrix[1][2] = v.z;
        camera.inverseViewMatrix[2][0] = w.x;
        camera.inverseViewMatrix[2][1] = w.y;
        camera.inverseViewMatrix[2][2] = w.z;
        camera.inverseViewMatrix[3][0] = transform.position.x;
        camera.inverseViewMatrix[3][1] = transform.position.y;
        camera.inverseViewMatrix[3][2] = transform.position.z;
    }

    CameraComponent &CameraSystem::getMainCamera() {
        CameraComponent *fallBackCamera{nullptr};
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (!hasCamera(entity))
                continue;

            auto &camera = getCamera(entity);
            if (camera.isMain)
                return camera;

            if (fallBackCamera == nullptr)
                fallBackCamera = &camera;
        }

        if (fallBackCamera != nullptr)
            fallBackCamera->isMain = true;

        return *fallBackCamera;
    }

    void CameraSystem::update(FrameInfo &frameInfo) {
        CameraComponent *mainCamera{nullptr};
        CameraComponent *fallbackCamera{nullptr};

        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (!hasCamera(entity))
                continue;

            auto &camera = getCamera(entity);
            auto &cameraTransform = m_ecs.getComponent<TransformComponent>(entity);
            setViewYXZ(camera, cameraTransform);

            // setOthrographicProjection(-frameInfo.aspect, frameInfo.aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            setPerspectiveProjection(camera, glm::radians(50.0f), frameInfo.aspect, 0.1f, 100.0f);

            if (mainCamera != nullptr)
                continue;

            if (camera.isMain)
                mainCamera = &camera;

            if (fallbackCamera == nullptr)
                fallbackCamera = &camera;
        }

        if (mainCamera == nullptr)
            mainCamera = fallbackCamera;

        assert(mainCamera != nullptr && "No cameras available");

        frameInfo.camera = mainCamera;
        frameInfo.ubo.projection = mainCamera->projectionMatrix;
        frameInfo.ubo.view = mainCamera->viewMatrix;
        frameInfo.ubo.inverseView = mainCamera->inverseViewMatrix;
    }

    bool CameraSystem::hasCamera(Entity cameraEntity) {
        return m_ecs.hasComponent<CameraComponent>(cameraEntity);
    }

    CameraComponent &CameraSystem::getCamera(Entity cameraEntity) {
        assert(hasCamera(cameraEntity) && "Camera does not exist");
        return m_ecs.getComponent<CameraComponent>(cameraEntity);
    }
}
