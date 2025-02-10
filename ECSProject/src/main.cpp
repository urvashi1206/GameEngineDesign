#include <iostream>

#include "ecs/ECSCoordinator.hpp"
#include "ecs/Components.hpp"
#include "Systems.hpp"

#include <raylib.h>

int main() {
    // 1. Create the ECS Coordinator
    ECSCoordinator ecs;

    // 2. Register all component types
    ecs.RegisterComponent<Position>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<Render>();
    //ecs.RegisterComponent<Sprite>();
    ecs.RegisterComponent<Health>();
    ecs.RegisterComponent<Collider>();
    ecs.RegisterComponent<Input>();

    // 3. Create player entity
    Entity player = ecs.CreateEntity();
    ecs.AddComponent<Position>(player, { 0.0f, 0.0f, 0.0f });
    ecs.AddComponent<Velocity>(player, { 0.0f, 0.0f, 0.0f });
    ecs.AddComponent<Render>(player, { RED, 1.0f });  // Fallback circle
    ecs.AddComponent<Collider>(player, { 1.0f, 1.0f, 1.0f }); 
    ecs.AddComponent<Input>(player, {});  // So we can move with arrow keys
    ecs.AddComponent<Health>(player, { 100, 100 });

    // 4. Add enemy entity
    Entity enemy = ecs.CreateEntity();
    ecs.AddComponent<Position>(enemy, { 5.0f, 0.0f, 0.0f });
    ecs.AddComponent<Collider>(enemy, { 1.0f, 1.0f, 1.0f });
    ecs.AddComponent<Render>(enemy, { BLUE, 1.0f });
    ecs.AddComponent<Health>(enemy, { 50, 50 });

    // Set up a 3D camera
    Camera3D camera   = { 0 };
    camera.position   = Vector3{ 10.0f, 10.0f, 10.0f };
    camera.target     = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up         = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // 5. Initialize Raylib window
    InitWindow(800, 600, "ECS + Raylib Example");
    SetTargetFPS(60);

    // 6. Main game loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        InputSystem(ecs, dt);
        MovementSystem(ecs, dt);
        CollisionSystem(ecs, dt);
        RenderSystem(ecs, camera);
    }

    CloseWindow();
    return 0;
}
