#pragma once

#include <raylib.h> 

struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

struct Sprite {
    Texture2D texture;
    float rotation;
    float scale;
};

struct Collider {
    float width;
    float height;
};

struct Health {
    int current;
    int max;
};

struct Input {
    bool moveLeft;
    bool moveRight;
    bool moveUp;
    bool moveDown;
};

// Add other components as needed, e.g., Health, Sprite, etc.
