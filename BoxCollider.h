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
	virtual std::vector<Vector> EPA_GetAlignedFace(const Vector& direction, Vector& out_faceNormal) const override;

	virtual Matrix4x4 GetInertiaTensor(float mass) const override;

	Vector GetCenter() const { return transform->LocalToWorld_Point(center); };
	Vector GetHalfSize() const { return transform->LocalToWorld_Point(halfSize); };

	Vector GetWorldMin() const;
	Vector GetWorldMax() const;

	//Transform GetWorldMaxT() const { return Transform(transform->LocalToWorld(center + halfSize),); };

	//Entity* GetDebugWireframe() const override;
};