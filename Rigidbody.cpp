#include "Rigidbody.h"

#include <iostream>

#include "ConvexCollider.h"
#include "PhysicsSubsystem.h"

#include "DirectXMath.h"

Rigidbody::Rigidbody(Vector gravity, bool isStatic, float mass) : 
	gravity(gravity), isStatic(isStatic), mass(mass), velocity(), angularVelocity(), netForce(), netTorque()
{

}
Rigidbody::~Rigidbody()
{

}

void Rigidbody::UpdatePhysics(float deltaTime)
{
	// Minimum velocity allowed for a body's velocity to be considered non-zero. Basically acts as a global static coefficient of friction.
	static const float VELOCITY_THRESHOLD = 0.02f;

	// Velocity threshold to prevent small movements from accumulating
    if(velocity.GetMagnitude() < VELOCITY_THRESHOLD)
        velocity = Vector::Zero();
    if(angularVelocity.GetMagnitude() < VELOCITY_THRESHOLD)
        angularVelocity = Vector::Zero();

	GetTransform()->MoveAbsolute(velocity * deltaTime);
	GetTransform()->RotateAxisAngle(angularVelocity * deltaTime);

	velocity += netForce * deltaTime;
	angularVelocity += netTorque * deltaTime;

	netForce = Vector();
	netTorque = Vector();
}

void Rigidbody::AddForce(Vector force)
{
	if(isStatic)
		return;

	netForce += force;
}
void Rigidbody::AddTorque(Vector axisAngle)
{
	if(isStatic)
		return;

	netTorque += axisAngle;
}
void Rigidbody::ApplyGravity()
{
	AddForce(gravity);
}

void Rigidbody::ApplyImpulse(const Vector& impulse, const Vector& location)
{
	if(isStatic)
		return;

	// Apply linear impulse
	velocity += impulse / mass;
	
	Vector r = location - ((ConvexCollider*) GetCollider())->GetCenter();

	// Apply angular impulse
	angularVelocity += GetInertiaTensor().Inverse() * r.Cross(impulse);
}

void Rigidbody::Initialize()
{
	PhysicsSubsystem::AddRigidbody(this);
}
void Rigidbody::Update(float deltaTime)
{

}

Matrix4x4 Rigidbody::GetInertiaTensor() const
{
	Matrix4x4 inertiaTensor = GetCollider()->GetInertiaTensor(mass);

	return entity->GetComponent<Transform>()->GetRotationMatrix() * (inertiaTensor * entity->GetComponent<Transform>()->GetRotationMatrix().Transpose());
}