#include "SphereCollider.h"
#include "Debug.h"

SphereCollider::SphereCollider(Vector offset, float radius, bool showDebug) : 
	offset(offset), radius(radius), showDebug(showDebug)
{
    colliderType = ColliderType::Convex;

    if(showDebug)
        Debug::CreateDebugSphere(GetTransform(), offset, radius);
}

Vector SphereCollider::GJK_Support(const Vector& direction) const
{
    return GetTransform()->GetLocation() + offset + direction.Normalized() * radius;
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