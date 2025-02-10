#pragma once

#include <raylib.h> 

struct Position {
   /* float x;
    float y;*/
	Vector3 pos;
};

struct Velocity {
    /*float dx;
    float dy;*/
	Vector3 vel;
};

//struct Sprite {
//    Texture2D texture;
//    float rotation;
//    float scale;
//};

struct Render {
    Color color;
    float size;
};

struct Collider {
    float width;
    float height;
	float depth;
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
