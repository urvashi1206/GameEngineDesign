#include "Systems.hpp"
#include <raylib.h>
#include <raymath.h>
#include <cmath>

#include "ecs/Components.hpp"
#include "ecs/ECSCoordinator.hpp"

void MovementSystem(ECSCoordinator& ecs, float dt) {
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (ecs.HasComponent<Transform3D>(e) && ecs.HasComponent<Velocity>(e)) {
            auto& transform = ecs.GetComponent<Transform3D>(e);
            auto& velocity = ecs.GetComponent<Velocity>(e);

            transform.position.x += velocity.vel.x * dt;
            transform.position.y += velocity.vel.y * dt;
			transform.position.z += velocity.vel.z * dt;
        }
    }
}

void InputSystem(ECSCoordinator& ecs, float dt) {
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (ecs.HasComponent<Input>(e) && ecs.HasComponent<Velocity>(e)) {
            auto& inp = ecs.GetComponent<Input>(e);
            auto& vel = ecs.GetComponent<Velocity>(e);

            inp.moveLeft = IsKeyDown(KEY_LEFT);
            inp.moveRight = IsKeyDown(KEY_RIGHT);
            inp.moveUp = IsKeyDown(KEY_UP);
            inp.moveDown = IsKeyDown(KEY_DOWN);

            const float speed = 20.0f;
            vel.vel = { 0.0f, 0.0f, 0.0f };

            if (inp.moveLeft)  vel.vel.x = -speed;
            if (inp.moveRight) vel.vel.x = +speed;
            if (inp.moveUp)    vel.vel.z = -speed;
            if (inp.moveDown)  vel.vel.z = +speed;
        }
    }
}

void CollisionSystem(ECSCoordinator& ecs, float dt) {
    for (Entity a = 0; a < MAX_ENTITIES; a++) {
        if (!ecs.HasComponent<Transform3D>(a) || !ecs.HasComponent<Collider>(a)) continue;

        auto& transformA = ecs.GetComponent<Transform3D>(a);
        auto& colliderA = ecs.GetComponent<Collider>(a);

        float halfWidthA = colliderA.width * 0.5f;
        float halfHeightA = colliderA.height * 0.5f;
        float halfDepthA = colliderA.depth * 0.5f;

        // A's bounding box
        float leftA = transformA.position.x - halfWidthA;
        float rightA = transformA.position.x + halfWidthA;
        float bottomA = transformA.position.y - halfHeightA;
        float topA = transformA.position.y + halfHeightA;
        float backA = transformA.position.z - halfDepthA;
        float frontA = transformA.position.z + halfDepthA;

        for (Entity b = a + 1; b < MAX_ENTITIES; b++) {
            if (!ecs.HasComponent<Transform3D>(b) || !ecs.HasComponent<Collider>(b)) continue;

            auto& transformB = ecs.GetComponent<Transform3D>(b);
            auto& colliderB = ecs.GetComponent<Collider>(b);

            float halfWidthB = colliderB.width * 0.5f;
            float halfHeightB = colliderB.height * 0.5f;
            float halfDepthB = colliderB.depth * 0.5f;

            // B's bounding box
            float leftB = transformB.position.x - halfWidthB;
            float rightB = transformB.position.x + halfWidthB;
			float bottomB = transformB.position.y - halfHeightB;
			float topB = transformB.position.y + halfHeightB;
			float backB = transformB.position.z - halfDepthB;
			float frontB = transformB.position.z + halfDepthB;

            // Check for overlap in x, y, and z axes
            bool overlap = (leftA < rightB && rightA > leftB &&
                bottomA < topB && topA > bottomB &&
                backA < frontB && frontA > backB);

            if (overlap) {
                TraceLog(LOG_INFO, "Collision: Entity %d with Entity %d", a, b);
            }
        }
    }
}

void RenderSystem(ECSCoordinator& ecs, Camera3D& camera) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);
    DrawGrid(10, 1.0f);

    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        bool hasTransform = ecs.HasComponent<Transform3D>(e);
        bool hasMesh = ecs.HasComponent<MeshData>(e);
        bool hasRenderer = ecs.HasComponent<Render>(e);

        if (hasTransform && hasMesh && hasRenderer) 
        {
            auto& pos = ecs.GetComponent<Transform3D>(e);
            auto& ren = ecs.GetComponent<Render>(e);
			auto& mesh = ecs.GetComponent<MeshData>(e);

            DrawSphere(pos.position, ren.size, ren.color);

			if (ren.size > 0.0f) {
				DrawSphereWires(pos.position, ren.size, 16, 16, BLACK);
			}
        }
    }

    EndMode3D();
    EndDrawing();
}