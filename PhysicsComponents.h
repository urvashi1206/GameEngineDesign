#pragma once

#include "Vector.h"

struct Transform
{
	Vector location;
	Vector rotation;
	Vector scale;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	// Flag for if the stored world matrix needs to be updated to reflect transformations
	bool isWorldMatrixDirty = true;
};

struct Rigidbody
{
	bool isStatic;

	float mass = 1;
	float bounciness = 0;
	float staticFriction = 0.3f;
	float dynamicFriction = 0.5f;

	Vector gravity;

	Vector velocity;
	Vector angularVelocity;

	Vector netForce;
	Vector netTorque;
};

enum class EColliderType
{
	None,
	Box,
	Sphere
};
struct Collider
{
	EColliderType colliderType;

	Vector center;
};
struct BoxCollider : public Collider
{
	Vector halfSize;
};
struct SphereCollider : public Collider
{
	float radius;
};