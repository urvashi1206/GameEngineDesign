#pragma once

#include "ecs/ECSCoordinator.hpp"


void MovementSystem(ECSCoordinator& ecs, float dt);
void RenderSystem(ECSCoordinator& ecs);
void InputSystem(ECSCoordinator& ecs, float dt);
void CollisionSystem(ECSCoordinator& ecs, float dt);
