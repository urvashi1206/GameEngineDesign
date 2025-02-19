#include "Rigidbody.h"

#include <iostream>

Rigidbody::Rigidbody(Transform* transform, Collider* collider, Vector gravity, bool isStatic) : 
	transform(transform), collider(collider), gravity(gravity), isStatic(isStatic), velocity(), angularVelocity(), netForce(), netTorque()
{

}
Rigidbody::~Rigidbody()
{

}

void Rigidbody::UpdatePhysics(float deltaTime)
{
	//transform.MoveAbsolute(velocity.x, velocity.y, deltaTime);
	transform->MoveAbsolute(velocity * deltaTime);
	transform->Rotate(angularVelocity * deltaTime);
	
	if(!isStatic)
	{
		auto k = transform->GetLocation();
		/*std::cout << "Velocity: " << velocity.x << " " << velocity.y << " " << velocity.z << std::endl;
		std::cout << "Position: " << k.x << " " << k.y << " " << k.z << std::endl;*/
	}

	velocity += netForce * deltaTime;
	angularVelocity += netTorque * deltaTime;

	//transform.MoveAbsolute(1, 1, 1);

	netForce = Vector();
	netTorque = Vector();
}

void Rigidbody::AddForce(Vector force)
{
	if(isStatic)
		return;

	netForce += force;
}
void Rigidbody::AddTorque(Vector force, float distance)
{
	if(isStatic)
		return;

	netTorque += force * distance;
}
void Rigidbody::ApplyGravity()
{
	AddForce(gravity);
}