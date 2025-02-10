#pragma once

#include <raylib.h>
#include "ecs/ECSCoordinator.hpp"


void MovementSystem(ECSCoordinator& ecs, float dt);
void RenderSystem(ECSCoordinator& ecs, Camera3D& camera);
void InputSystem(ECSCoordinator& ecs, float dt);
void CollisionSystem(ECSCoordinator& ecs, float dt);
