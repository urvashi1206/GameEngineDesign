#include <iostream>

#include "ecs/ECSCoordinator.hpp"
#include "ecs/Components.hpp"
#include "ecs/RenderComponent.hpp"
#include "Systems.hpp"

#include <raylib.h>

int main() {
    // 1. Create the ECS Coordinator
    ECSCoordinator ecs;

    // 2. Register all component types
    ecs.RegisterComponent<Position>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<Render>();
    ecs.RegisterComponent<Sprite>();
    ecs.RegisterComponent<Health>();
    ecs.RegisterComponent<Collider>();
    ecs.RegisterComponent<Input>();

    // 3. Create player entity
    Entity player = ecs.CreateEntity();
    ecs.AddComponent<Position>(player, { 100.f, 100.f });
    ecs.AddComponent<Velocity>(player, { 0.f, 0.f });
    ecs.AddComponent<Render>(player, { RED, 20.f });  // Fallback circle
    ecs.AddComponent<Collider>(player, { 20.f, 20.f }); 
    ecs.AddComponent<Input>(player, {});  // So we can move with arrow keys
    ecs.AddComponent<Health>(player, { 100, 100 });

    // 4. Add enemy entity
    Entity enemy = ecs.CreateEntity();
    ecs.AddComponent<Position>(enemy, { 300.f, 300.f });
    ecs.AddComponent<Collider>(enemy, { 40.f, 40.f });
    ecs.AddComponent<Render>(enemy, { BLUE, 40.f });
    ecs.AddComponent<Health>(enemy, { 50, 50 });

    // 5. Initialize Raylib window
    InitWindow(800, 600, "ECS + Raylib Example");
    SetTargetFPS(60);

    // 6. Main game loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Update systems in an order that makes sense
        InputSystem(ecs, dt);       // Read keyboard, update velocities
        MovementSystem(ecs, dt);    // Move entities
        CollisionSystem(ecs, dt);   // Check collisions
        RenderSystem(ecs);          // Draw everything
    }

    // Cleanup 
    CloseWindow();
    return 0;
}
