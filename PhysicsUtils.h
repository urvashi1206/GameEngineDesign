#pragma once

#include "PhysicsComponents.h"
#include "Matrix.h"

namespace TransformUtils
{
	static void MoveAbsolute(Transform& transform, Vector vector);
	static void MoveRelative(Transform& transform, Vector vector);
	static void Rotate(Transform& transform, Vector quaternion);
	static void RotateAxisAngle(Transform& transform, Vector axisAngle);
	static void RotateAxisAngle(Transform& transform, Vector axis, float angle);
	static void RotatePitchYawRoll(Transform& transform, Vector pitchYawRoll);
	static void Scale(Transform& transform, Vector vector);

	static Vector GetPitchYawRoll(const Transform& transform);
	static Vector GetRight(const Transform& transform);
	static Vector GetUp(const Transform& transform);
	static Vector GetForward(const Transform& transform);
	static DirectX::XMFLOAT4X4 GetWorldMatrix(Transform& transform);
	static DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix(Transform& transform);

	static Matrix4x4 GetRotationMatrix(const Transform& transform);

	static void SetLocation(Transform& transform, Vector location);
	static void SetRotation(Transform& transform, Vector quaternion);
	static void SetRotationPitchYawRoll(Transform& transform, Vector pitchYawRoll);
	static void SetScale(Transform& transform, Vector scale);

	static Vector LocalToWorld_Point(Transform& transform, const Vector& vector, bool includeScale = false);
	static Vector LocalToWorld_Direction(Transform& transform, const Vector& vector, bool includeScale = false);
	static Vector WorldToLocal_Point(Transform& transform, const Vector& vector);
	static Vector WorldToLocal_Direction(Transform& transform, const Vector& vector);
}

namespace ColliderUtils
{
	Matrix4x4 GetInertiaTensor(const Collider& collider, float mass);
}

namespace RigidbodyUtils
{
	void UpdatePhysics(Transform& transform, Rigidbody& rigidbody, float deltaTime);

	void AddForce(Rigidbody& rigidbody, Vector force);
	void AddTorque(Rigidbody& rigidbody, Vector axisAngle);
	void ApplyGravity(Rigidbody& rigidbody);

	void ApplyImpulse(const Transform& transform, const Collider& collider, Rigidbody& rigidbody, const Vector& impulse, const Vector& location);

	float GetMass(const Rigidbody& rigidbody);
	float GetInverseMass(const Rigidbody& rigidbody);

	Matrix4x4 GetInertiaTensor(const Transform& transform, const Collider& collider, const Rigidbody& rigidbody);
}