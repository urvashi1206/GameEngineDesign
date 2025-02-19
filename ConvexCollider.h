#pragma once

#include "Collider.h"

#include "Vector.h"

class ConvexCollider : public Collider
{
public:
	virtual Vector GJK_Support(const Vector& direction) const = 0;
};