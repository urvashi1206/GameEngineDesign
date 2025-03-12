#pragma once

#include "ConvexCollider.h"

class SphereCollider : public ConvexCollider
{
private:
	Vector offset;
	float radius;

	bool showDebug;

public:
	SphereCollider(Transform* transform, Vector offset, float radius, bool showDebug = false);

	virtual Vector GJK_Support(const Vector& direction) const override;
	virtual std::vector<Vector> EPA_GetAlignedFace(const Vector& direction, Vector& out_faceNormal) const override;

	virtual Matrix4x4 GetInertiaTensor(float mass) const override;
};