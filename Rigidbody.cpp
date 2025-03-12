#include "Rigidbody.h"

#include <iostream>

#include "DirectXMath.h"

Rigidbody::Rigidbody(Transform* transform, Collider* collider, Vector gravity, bool isStatic, float mass) : 
	transform(transform), collider(collider), gravity(gravity), isStatic(isStatic), mass(mass), velocity(), angularVelocity(), netForce(), netTorque()
{

}
Rigidbody::~Rigidbody()
{

}

void Rigidbody::UpdatePhysics(float deltaTime)
{
	/*DirectX::XMFLOAT3 f3_axis(1, 1, 1);
	DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&f3_axis), 1);

	DirectX::XMFLOAT4 f4_q;
	DirectX::XMStoreFloat4(&f4_q, q);
	angularVelocity = f4_q;*/

	//transform.MoveAbsolute(velocity.x, velocity.y, deltaTime);
	transform->MoveAbsolute(velocity * deltaTime);
	//transform->RotateAxisAngle(Vector(1, 1, 1), deltaTime);
	transform->RotateAxisAngle(angularVelocity * deltaTime);
	
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

Matrix4x4 Rigidbody::GetInertiaTensor() const
{
	Matrix4x4 inertiaTensor = collider->GetInertiaTensor(mass);

	return transform->GetRotationMatrix() * (inertiaTensor * transform->GetRotationMatrix().Transpose());
}