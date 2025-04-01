#pragma once

#include <vector>

#include "Vector.h"
#include "Transform.h"
#include "Entity.h"

enum class ColliderType : uint8_t
{
	None		= 0b00000000,
	Convex		= 0b00000001,
	Composite	= 0b00000010
};

class Collider : public Component
{
protected:
	ColliderType colliderType;

public:
	Collider();
	~Collider();

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

public:
	virtual Vector GetCenter() const = 0;
	virtual Matrix4x4 GetInertiaTensor(float mass) const = 0;

	//virtual void CreateDebugWireframe(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) const = 0;

	ColliderType GetColliderType() const { return colliderType; };

	//virtual Entity* GetDebugWireframe() const { return nullptr; };

	Transform* GetTransform() const { return entity->GetComponent<Transform>(); };
};