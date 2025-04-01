#pragma once

#include "Vector.h"
#include "Transform.h"
#include "Collider.h"

class Rigidbody : public Component
{
private:
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
	Rigidbody(Vector gravity = Vector(0, -9.81f, 0), bool isStatic = false, float mass = 1);
	~Rigidbody();

	void UpdatePhysics(float deltaTime);

	void AddForce(Vector force);
	void AddTorque(Vector axisAngle);
	void ApplyGravity();

	void ApplyImpulse(const Vector& impulse, const Vector& location);

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

public:
	float GetMass() const { return isStatic ? FLT_MAX : mass; }; // Static bodies should be treated as having infinite mass
	float GetInverseMass() const { return isStatic ? 0 : 1.0f / mass; };
	Vector GetVelocity() const { return velocity; };
	Vector GetAngularVelocity() const { return angularVelocity; };
	
	float GetStaticFriction() const { return 0.3f; };
	float GetDynamicFriction() const { return 0.5f; };

	Collider* GetCollider() const { return entity->GetComponent<Collider>(); };

	Transform* GetTransform() const { return entity->GetComponent<Transform>(); };

	Matrix4x4 GetInertiaTensor() const;

	void SetVelocity(Vector value) { if(!isStatic) velocity = value; };
	void SetAngularVelocity(Vector axisAngle) { if(!isStatic) angularVelocity = axisAngle; };

	//friend void PhysicsSubsystem::Update(float deltaTime);
};