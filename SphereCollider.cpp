#include "SphereCollider.h"

SphereCollider::SphereCollider(Transform* transform, Vector offset, float radius, bool showDebug) : 
	offset(offset), radius(radius), showDebug(showDebug)
{
    this->transform = transform;
    colliderType = ColliderType::Sphere;
}

Vector SphereCollider::GJK_Support(const Vector& direction) const
{
    return transform->GetLocation() + offset + direction.Normalized() * radius;
}