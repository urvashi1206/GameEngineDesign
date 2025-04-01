#pragma once

#include "Collider.h"

#include "ConvexCollider.h"

class CompositeCollider : public Collider
{
protected:
	std::vector<ConvexCollider*> colliders;

public:
	CompositeCollider();
	CompositeCollider(std::vector<ConvexCollider*> colliders);
	~CompositeCollider();

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

	void SetEntity(Entity* entity) override;

public:
	Vector GetCenter() const override;
	virtual Matrix4x4 GetInertiaTensor(float mass) const override;

	std::vector<ConvexCollider*> GetColliders() const { return colliders; };
};