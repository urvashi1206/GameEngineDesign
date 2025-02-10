#include "Systems.hpp"
#include <raylib.h>
#include <cmath>

#include "ecs/Components.hpp"

void MovementSystem(ECSCoordinator& ecs, float dt) {
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (ecs.HasComponent<Position>(e) && ecs.HasComponent<Velocity>(e)) {
            auto& pos = ecs.GetComponent<Position>(e);
            auto& vel = ecs.GetComponent<Velocity>(e);

            pos.pos.x += vel.vel.x * dt;
            pos.pos.y += vel.vel.y * dt;
			pos.pos.z += vel.vel.z * dt;
        }
    }
}

void InputSystem(ECSCoordinator& ecs, float dt) {
    // For simplicity, let's assume we have only one "player" entity
    // In a real game, you might store or look up the player's entity ID
    // or apply to all entities that have Input + Velocity.
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (ecs.HasComponent<Input>(e) && ecs.HasComponent<Velocity>(e)) {
            auto& inp = ecs.GetComponent<Input>(e);
            auto& vel = ecs.GetComponent<Velocity>(e);

            // Check Raylib input
            if (IsKeyDown(KEY_LEFT))  inp.moveLeft  = true; else inp.moveLeft  = false;
            if (IsKeyDown(KEY_RIGHT)) inp.moveRight = true; else inp.moveRight = false;
            if (IsKeyDown(KEY_UP))    inp.moveUp    = true; else inp.moveUp    = false;
            if (IsKeyDown(KEY_DOWN))  inp.moveDown  = true; else inp.moveDown  = false;

            // Set velocity based on input; adjust speed as needed.
            const float speed = 1.0f;
            vel.vel = { 0.0f, 0.0f, 0.0f };

            // Convert input booleans to velocity
            if (inp.moveLeft)  vel.vel.x = -speed;
            if (inp.moveRight) vel.vel.x = +speed;
            if (inp.moveUp)    vel.vel.z = -speed;
            if (inp.moveDown)  vel.vel.z = +speed;
        }
    }
}

// 3) Collision System (AABB Checking)
void CollisionSystem(ECSCoordinator& ecs, float dt) {
    // We'll do a naive O(n^2) approach: check every entity vs. every other
    // In a real project, you might store a list of "collidable" entities or use a spatial partition.
    for (Entity a = 0; a < MAX_ENTITIES; a++) {
        if (!ecs.HasComponent<Position>(a) || !ecs.HasComponent<Collider>(a)) continue;

        auto& posA = ecs.GetComponent<Position>(a);
        auto& colA = ecs.GetComponent<Collider>(a);

        // A's bounding box
        float leftA   = posA.pos.x;
        float rightA  = posA.pos.x + colA.width;
        float bottomA = posA.pos.y;
        float topA    = posA.pos.y + colA.height;
		float backA   = posA.pos.z;
		float frontA  = posA.pos.z + colA.depth;

        for (Entity b = a + 1; b < MAX_ENTITIES; b++) {
            if (!ecs.HasComponent<Position>(b) || !ecs.HasComponent<Collider>(b)) continue;

            auto& posB = ecs.GetComponent<Position>(b);
            auto& colB = ecs.GetComponent<Collider>(b);

            // B's bounding box
            float leftB   = posB.pos.x;
            float rightB  = posB.pos.x + colB.width;
            float bottomB = posB.pos.y;
            float topB    = posB.pos.y + colB.height;
			float backB   = posB.pos.z;
			float frontB  = posB.pos.z + colB.depth;

            // Check for overlap in x, y, and z axes
            bool overlap = (leftA < rightB && rightA > leftB &&
                bottomA < topB && topA > bottomB &&
                backA < frontB && frontA > backB);

            if (overlap) {
                // Resolve collision, apply damage, or simply log it
                // For example, we can just print:
                TraceLog(LOG_INFO, "Collision: Entity %d with Entity %d", a, b);

                // Possibly a "DamageSystem" or immediate response here
            }
        }
    }
}

// 4) Render System
//   - Draw entities that have either: 
//     - A Sprite, or 
//     - A Render data with just color & size
void RenderSystem(ECSCoordinator& ecs, Camera3D& camera) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (ecs.HasComponent<Position>(e) && ecs.HasComponent<Render>(e)) {
            auto& pos = ecs.GetComponent<Position>(e);
            auto& ren = ecs.GetComponent<Render>(e);
            DrawSphere(pos.pos, ren.size, ren.color);
            if (ecs.HasComponent<Collider>(e)) {
                auto& col = ecs.GetComponent<Collider>(e);
                DrawCubeWires(pos.pos, col.width, col.height, col.depth, BLACK);
            }
        }
    }

    EndMode3D();
    EndDrawing();
}