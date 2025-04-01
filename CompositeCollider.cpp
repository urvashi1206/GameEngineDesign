#include "CompositeCollider.h"

CompositeCollider::CompositeCollider()
{
	colliderType = ColliderType::Composite;
}
CompositeCollider::CompositeCollider(std::vector<ConvexCollider*> colliders) : 
	colliders(colliders)
{
	colliderType = ColliderType::Composite;
}
CompositeCollider::~CompositeCollider()
{
	for(ConvexCollider* c : colliders)
	{
		if(c) delete c;
		c = nullptr;
	}
}

void CompositeCollider::Initialize()
{
	for(ConvexCollider* c : colliders)
		c->Initialize();
}
void CompositeCollider::Update(float deltaTime)
{
	for(ConvexCollider* c : colliders)
		c->Update(deltaTime);
}

void CompositeCollider::SetEntity(Entity* entity)
{
	Component::SetEntity(entity);

	for(ConvexCollider* c : colliders)
		c->SetEntity(entity);
}

Vector CompositeCollider::GetCenter() const
{
	Vector sumCenters = Vector::Zero();
	for(ConvexCollider* c : colliders)
		sumCenters += c->GetCenter();

	return sumCenters / colliders.size();
}
Matrix4x4 CompositeCollider::GetInertiaTensor(float mass) const
{
	// TO-DO: CALCULATE COMPOSITE INERTIA TENSOR
	return Matrix4x4::Identity();
}