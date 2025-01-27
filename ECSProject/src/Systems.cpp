#include "Systems.hpp"
#include <raylib.h>
#include <cmath>

#include "ecs/Components.hpp"
#include "ecs/RenderComponent.hpp"

void MovementSystem(ECSCoordinator& ecs, float dt) {
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (ecs.HasComponent<Position>(e) && ecs.HasComponent<Velocity>(e)) {
            auto& pos = ecs.GetComponent<Position>(e);
            auto& vel = ecs.GetComponent<Velocity>(e);

            pos.x += vel.dx * dt;
            pos.y += vel.dy * dt;
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

            // Zero out old velocity
            vel.dx = 0.0f;
            vel.dy = 0.0f;

            // Check Raylib input
            if (IsKeyDown(KEY_LEFT))  inp.moveLeft  = true; else inp.moveLeft  = false;
            if (IsKeyDown(KEY_RIGHT)) inp.moveRight = true; else inp.moveRight = false;
            if (IsKeyDown(KEY_UP))    inp.moveUp    = true; else inp.moveUp    = false;
            if (IsKeyDown(KEY_DOWN))  inp.moveDown  = true; else inp.moveDown  = false;

            // Convert input booleans to velocity
            if (inp.moveLeft)  vel.dx = -100.0f;
            if (inp.moveRight) vel.dx = +100.0f;
            if (inp.moveUp)    vel.dy = -100.0f;
            if (inp.moveDown)  vel.dy = +100.0f;
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
        float leftA   = posA.x;
        float rightA  = posA.x + colA.width;
        float topA    = posA.y;
        float bottomA = posA.y + colA.height;

        for (Entity b = a + 1; b < MAX_ENTITIES; b++) {
            if (!ecs.HasComponent<Position>(b) || !ecs.HasComponent<Collider>(b)) continue;

            auto& posB = ecs.GetComponent<Position>(b);
            auto& colB = ecs.GetComponent<Collider>(b);

            // B's bounding box
            float leftB   = posB.x;
            float rightB  = posB.x + colB.width;
            float topB    = posB.y;
            float bottomB = posB.y + colB.height;

            // Check for overlap
            bool overlap = (leftA < rightB && rightA > leftB &&
                            topA < bottomB && bottomA > topB);

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
void RenderSystem(ECSCoordinator& ecs) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        // If entity has a Sprite, draw that
        if (ecs.HasComponent<Position>(e) && ecs.HasComponent<Sprite>(e)) {
            auto& pos = ecs.GetComponent<Position>(e);
            auto& spr = ecs.GetComponent<Sprite>(e);

            DrawTextureEx(
                spr.texture,
                {pos.x, pos.y},
                spr.rotation,
                spr.scale,
                WHITE
            );
        }
        // Otherwise, if entity has a "Render" struct like your old circle,
        // or if it doesn't have a sprite but has color, you can draw a fallback
        else if (ecs.HasComponent<Position>(e) && ecs.HasComponent<Render>(e)) {
            auto& pos = ecs.GetComponent<Position>(e);
            auto& ren = ecs.GetComponent<Render>(e);
            DrawCircle((int)pos.x, (int)pos.y, ren.size, ren.color);
        }
    }

    EndDrawing();
}