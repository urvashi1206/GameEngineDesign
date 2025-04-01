#pragma once

#include "Entity.h"
#include "Rigidbody.h"
#include "Collider.h"

class PhysicsEntity //: public Entity
{
private:
	Rigidbody* rigidbody;
	Collider* collider;

public:
	PhysicsEntity(Rigidbody* rigidbody, Collider* collider = nullptr);
	~PhysicsEntity();

	void Update();
};