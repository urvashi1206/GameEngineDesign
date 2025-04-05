//
// Created by rohit on 4/4/2025.
//

#include "CameraSystem.hpp"

Minimal::CameraSystem::CameraSystem(ECSCoordinator &ecs): m_ecs(ecs) {}
void Minimal::CameraSystem::setOrthographicProjection(Entity cameraEntity, float left, float right, float top, float bottom, float near, float far) {}
void Minimal::CameraSystem::setPerspectiveProjection(Entity cameraEntity, float fovY, float aspect, float near, float far) {}
void Minimal::CameraSystem::update(FrameInfo &frameInfo, GlobalUBO &ubo) {}
