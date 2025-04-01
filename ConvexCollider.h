#pragma once

#include "Collider.h"

#include "Vector.h"

class ConvexCollider : public Collider
{
public:
	virtual Vector GJK_Support(const Vector& direction) const = 0;
	virtual std::vector<Vector> EPA_GetAlignedFace(const Vector& direction, Vector& out_faceNormal) const = 0;

	friend class CompositeCollider;
};