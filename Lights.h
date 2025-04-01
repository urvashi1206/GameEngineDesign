#pragma once

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

#include <DirectXMath.h>

struct Light
{
	int LightType;
	DirectX::XMFLOAT3 Direction;	// Directional, Spot
	float Range;					// Point, Spot
	DirectX::XMFLOAT3 Location;		// Point, Spot
	float Intensity;				// All
	DirectX::XMFLOAT3 Color;		// All
	float SpotInnerAngle;			// Spot: Inner cone angle (in radians)
	float SpotOuterAngle;			// Spot: Outer cone angle (in radians)
	DirectX::XMFLOAT2 Padding;		// Padding for alignment purposes
};