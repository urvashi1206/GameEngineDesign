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
};