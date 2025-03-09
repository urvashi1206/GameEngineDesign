#include <iostream>

#include "ecs/ECSCoordinator.hpp"
#include "ecs/Components.hpp"
#include "Systems.hpp"

#include <raylib.h>

int main() {
    ECSCoordinator ecs;

    ecs.RegisterComponent<Transform3D>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<MeshData>();
    ecs.RegisterComponent<Render>();
    ecs.RegisterComponent<Health>();
    ecs.RegisterComponent<Collider>();
    ecs.RegisterComponent<Input>();

    Entity player1 = ecs.CreateEntity();
    ecs.AddComponent<Transform3D>(player1, {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    });
    ecs.AddComponent<Velocity>(player1, { Vector3{0.0f, 0.0f, 0.0f} });
	ecs.AddComponent<MeshData>(player1, {
        std::vector<Vertex>{},
        std::vector<unsigned>{} 
    });
    ecs.AddComponent<Render>(player1, { 
        RED, 
        false, 
        1.0f 
    });
    ecs.AddComponent<Collider>(player1, { 2.0f, 2.0f, 2.0f }); 
    ecs.AddComponent<Input>(player1, { false, false, false, false });
    ecs.AddComponent<Health>(player1, { 100, 100 });

    Entity enemy = ecs.CreateEntity();
    ecs.AddComponent<Transform3D>(enemy, {
        Vector3{5.f, 0.f, 0.f},
        Vector3{0.f, 30.f, 0.f},
        Vector3{1.f, 2.f, 1.f}
    });
    ecs.AddComponent<MeshData>(enemy, {
        std::vector<Vertex>{},
        std::vector<unsigned>{}
    });
    ecs.AddComponent<Render>(enemy, {
        BLUE,
		false,
        1.5f
    });
    ecs.AddComponent<Collider>(enemy, { 2.0f, 3.0f, 2.0f });
    ecs.AddComponent<Health>(enemy, { 50, 50 });

	Camera3D camera = { 0 };
    camera.position = Vector3{ 10.0f, 10.0f, 10.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    InitWindow(800, 600, "ECS + Raylib Example");
    SetTargetFPS(60);

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
