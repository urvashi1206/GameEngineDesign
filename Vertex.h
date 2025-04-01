#pragma once

#include <memory>

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 position;	    // The local position of the vertex
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uv;

	bool operator==(const Vertex& other) const;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(const Vertex& v) const {
            return hash<float>()(v.position.x) ^ hash<float>()(v.position.y) ^
                hash<float>()(v.position.z) ^ hash<float>()(v.normal.x) ^
                hash<float>()(v.normal.y) ^ hash<float>()(v.normal.z) ^
                hash<float>()(v.uv.x) ^ hash<float>()(v.uv.y);
        }
    };
}