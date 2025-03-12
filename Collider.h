#pragma once

#include <vector>

#include "Vector.h"
#include "Transform.h"
#include "Entity.h"

enum class ColliderType : uint8_t
{
	None	= 0b00000000,
	Box		= 0b00000001,
	Sphere	= 0b00000010
};

class Collider
{
protected:
	ColliderType colliderType;

public:
	Transform* transform;

public:
	Collider();
	Collider(Transform* transform);
	~Collider();

	virtual Matrix4x4 GetInertiaTensor(float mass) const = 0;

	//virtual void CreateDebugWireframe(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) const = 0;

	ColliderType GetColliderType() const { return colliderType; };

	//virtual Entity* GetDebugWireframe() const { return nullptr; };
};