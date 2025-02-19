#pragma once

#include "ConvexCollider.h"

class BoxCollider : public ConvexCollider
{
private:
	Vector center;
	Vector halfSize;

	Vector localMin;
	Vector localMax;

public:
	BoxCollider();
	BoxCollider(Transform* transform, Vector center, Vector halfSize, bool showDebug = false);
	~BoxCollider();

	//void CreateDebugWireframe(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) const override;

	virtual Vector GJK_Support(const Vector& direction) const override;

	Vector GetCenter() const { return transform->LocalToWorld(center); };
	Vector GetHalfSize() const { return transform->LocalToWorld(halfSize); };

	Vector GetWorldMin() const;
	Vector GetWorldMax() const;

	//Transform GetWorldMaxT() const { return Transform(transform->LocalToWorld(center + halfSize),); };

	//Entity* GetDebugWireframe() const override;
};