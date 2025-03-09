#pragma once

#include <raylib.h> 
#include <vector>

struct Transform3D {
    Vector3 position{ 0.0f, 0.0f, 0.0f };
	Vector3 rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 scale{ 1.0f, 1.0f, 1.0f };
};

struct Vertex {
	Vector3 position;
	Vector2 texCoord;
	Vector3 normal;
};

struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
};

struct Velocity {
	Vector3 vel;
};

struct Render {
	Color color{ 255, 255, 255, 255 };
	bool wireframe = false;
	float size = 1.0f;
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
