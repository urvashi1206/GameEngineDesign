#include "SphereCollider.h"
#include "Debug.h"

SphereCollider::SphereCollider(Transform* transform, Vector offset, float radius, bool showDebug) : 
	offset(offset), radius(radius), showDebug(showDebug)
{
    this->transform = transform;
    colliderType = ColliderType::Sphere;

    if(showDebug)
        Debug::CreateDebugSphere(transform, offset, radius);
}

Vector SphereCollider::GJK_Support(const Vector& direction) const
{
    return transform->GetLocation() + offset + direction.Normalized() * radius;
}
std::vector<Vector> SphereCollider::EPA_GetAlignedFace(const Vector& direction, Vector& out_faceNormal) const
{
    out_faceNormal = direction;

    return { GJK_Support(direction) };
}

Matrix4x4 SphereCollider::GetInertiaTensor(float mass) const
{
    return Matrix4x4::Identity() * ((2.0f / 3) * mass * radius * radius);
}