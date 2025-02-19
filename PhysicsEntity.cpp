#include "PhysicsEntity.h"

PhysicsEntity::PhysicsEntity(Rigidbody* rigidbody, Collider* collider) : 
	rigidbody(rigidbody), collider(collider)
{

}
PhysicsEntity::~PhysicsEntity()
{

}

void PhysicsEntity::Update()
{
	//if(collider)
	//{
		//collider->TestCollision();
	//}
}