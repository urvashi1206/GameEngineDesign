#pragma once

#include "Vector.h"
#include "Transform.h"
#include "Collider.h"

class Rigidbody
{
private:
	Transform* transform;
	Collider* collider;

	Vector velocity;
	Vector angularVelocity;

	Vector netForce;
	Vector netTorque;
	
	Vector gravity;

public:
	bool isStatic;

	float mass = 1;
	float bounciness = 0;

public:
	Rigidbody(Transform* transform, Collider* collider, Vector gravity = Vector(0, -9.81f, 0), bool isStatic = false, float mass = 1);
	~Rigidbody();

	void UpdatePhysics(float deltaTime);

	void AddForce(Vector force);
	void AddTorque(Vector axisAngle);
	void ApplyGravity();

	float GetMass() const { return isStatic ? FLT_MAX : mass; }; // Static bodies should be treated as having infinite mass
	Vector GetVelocity() const { return velocity; };
	Vector GetAngularVelocity() const { return angularVelocity; };

	Collider* GetCollider() const { return collider; };
	//Vector GetMin() { return Vector(transform.GetLocation().x + collider.min.x, transform.GetLocation().y + collider.min.y, transform.GetLocation().z + collider.min.z); };
	//Vector GetMax() { return Vector(transform.GetLocation().x + collider.max.x, transform.GetLocation().y + collider.max.y, transform.GetLocation().z + collider.max.z); };

	Transform* GetTransform() const { return transform; };

	Matrix4x4 GetInertiaTensor() const;

	void SetVelocity(Vector value) { if(!isStatic) velocity = value; };
	void SetAngularVelocity(Vector axisAngle) { if(!isStatic) angularVelocity = axisAngle; };

	//friend void PhysicsSubsystem::Update(float deltaTime);
};