#include "BoxCollider.h"

#include "Debug.h"

BoxCollider::BoxCollider()
{
	colliderType = ColliderType::Box;
}
BoxCollider::BoxCollider(Transform* transform, Vector center, Vector halfSize, bool showDebug) : 
	center(center), halfSize(halfSize)
{
	this->transform = transform;
	colliderType = ColliderType::Box;

	if(showDebug)
	{
		Debug::CreateDebugBox(transform, Vector(), halfSize);

		//Debug::CreateDebugBox(, );
	}
}
BoxCollider::~BoxCollider()
{
	
}

Vector BoxCollider::GJK_Support(const Vector& direction) const
{
	Vector localDirection = transform->WorldToLocal_Direction(direction);

	Vector min = center - halfSize;
	Vector max = center + halfSize;
	Vector supportLocal(
		localDirection.x >= 0.0f ? max.x : min.x,
		localDirection.y >= 0.0f ? max.y : min.y,
		localDirection.z >= 0.0f ? max.z : min.z);

	return transform->LocalToWorld(supportLocal);
}

Vector BoxCollider::GetWorldMin() const
{
	Vector w = transform->LocalToWorld(center - halfSize);
	Debug::CreateWireframe_Temp(w, Vector(), Vector(0.125f, 0.125f, 0.125f));
	return w;
}
Vector BoxCollider::GetWorldMax() const
{
	Vector w = transform->LocalToWorld(center + halfSize);
	Debug::CreateWireframe_Temp(w, Vector(), Vector(0.125f, 0.125f, 0.125f));
	return w;
}

//Entity* BoxCollider::GetDebugWireframe() const
//{
//	debugWireframe->GetTransform()->SetScale(halfSize.x * 2, halfSize.y * 2, halfSize.z * 2);
//	debugWireframe->GetTransform()->SetRotation(transform->GetPitchYawRoll());
//	debugWireframe->GetTransform()->SetLocation(transform->GetLocation());
//
//	return debugWireframe;
//}