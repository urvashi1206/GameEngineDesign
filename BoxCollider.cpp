#include "BoxCollider.h"

#include <vector>

#include "Debug.h"

BoxCollider::BoxCollider()
{
	colliderType = ColliderType::Convex;
}
BoxCollider::BoxCollider(Vector center, Vector halfSize, bool showDebug) : 
	center(center), halfSize(halfSize), showDebug(showDebug)
{
	colliderType = ColliderType::Convex;

	//if(showDebug)
		//Debug::CreateDebugBox(GetTransform(), center, halfSize);
}
BoxCollider::~BoxCollider()
{
	
}

Vector BoxCollider::GJK_Support(const Vector& direction) const
{
	Transform* transform = GetTransform();

	Vector min = center - halfSize;
	Vector max = center + halfSize;
	std::vector<Vector> vertices = 
	{
		transform->LocalToWorld_Point(Vector(min.x, min.y, min.z)),
		transform->LocalToWorld_Point(Vector(min.x, min.y, max.z)),
		transform->LocalToWorld_Point(Vector(min.x, max.y, min.z)),
		transform->LocalToWorld_Point(Vector(min.x, max.y, max.z)),
		transform->LocalToWorld_Point(Vector(max.x, min.y, min.z)),
		transform->LocalToWorld_Point(Vector(max.x, min.y, max.z)),
		transform->LocalToWorld_Point(Vector(max.x, max.y, min.z)),
		transform->LocalToWorld_Point(Vector(max.x, max.y, max.z))
	};

	Vector supportPoint;
	float maxDot = 0;
	for(Vector vertex : vertices)
	{
		float dot = direction.Dot(vertex - GetCenter());
		if(dot > maxDot)
		{
			supportPoint = vertex;
			maxDot = dot;
		}
	}

	return supportPoint;
}
std::vector<Vector> BoxCollider::EPA_GetAlignedFace(const Vector& direction, Vector& out_faceNormal) const
{
	Transform* transform = GetTransform();

	std::vector<Vector> face;

	Vector dominant;
	Vector u;
	Vector v;
	float halfSizeDominant = 0;
	float halfSizeU = 0;
	float halfSizeV = 0;
	float dotDominant = 0;

	float dotRight = direction.Dot(transform->GetRight());
	if(abs(dotRight) > abs(dotDominant))
	{
		dominant = (dotRight > 0) ? transform->GetRight() : -transform->GetRight();
		u = transform->GetForward();
		v = transform->GetUp();

		halfSizeDominant = halfSize.x;
		halfSizeU = halfSize.z;
		halfSizeV = halfSize.y;
		
		dotDominant = dotRight;
	}
	float dotUp = direction.Dot(transform->GetUp());
	if(abs(dotUp) > abs(dotDominant))
	{
		dominant = (dotUp > 0) ? transform->GetUp() : -transform->GetUp();
		u = transform->GetRight();
		v = transform->GetForward();

		halfSizeDominant = halfSize.y;
		halfSizeU = halfSize.x;
		halfSizeV = halfSize.z;

		dotDominant = dotUp;
	}
	float dotForward = direction.Dot(transform->GetForward());
	if(abs(dotForward) > abs(dotDominant))
	{
		dominant = (dotForward > 0) ? transform->GetForward() : -transform->GetForward();
		u = -transform->GetRight();
		v = transform->GetUp();

		halfSizeDominant = halfSize.z;
		halfSizeU = halfSize.x;
		halfSizeV = halfSize.y;

		dotDominant = dotForward;
	}

	Vector faceCenter = GetCenter() + dominant * halfSizeDominant;

	face.push_back(faceCenter - u * halfSizeU - v * halfSizeV);
	face.push_back(faceCenter + u * halfSizeU - v * halfSizeV);
	face.push_back(faceCenter + u * halfSizeU + v * halfSizeV);
	face.push_back(faceCenter - u * halfSizeU + v * halfSizeV);

	out_faceNormal = dominant;

	return face;
}

void BoxCollider::Initialize()
{

}
void BoxCollider::Update(float deltaTime)
{
	if(showDebug)
		Debug::CreateWireframe_Temp(GetCenter(), GetTransform()->GetRotation(), halfSize);
}

Matrix4x4 BoxCollider::GetInertiaTensor(float mass) const
{
	return Matrix4x4::Scale(Vector(
		(1.0f / 12) * mass * (pow(2 * halfSize.y, 2) + pow(2 * halfSize.z, 2)),
		(1.0f / 12) * mass * (pow(2 * halfSize.x, 2) + pow(2 * halfSize.y, 2)),
		(1.0f / 12) * mass * (pow(2 * halfSize.x, 2) + pow(2 * halfSize.z, 2)),
		1));
}

Vector BoxCollider::GetWorldMin() const
{
	Vector w = GetTransform()->LocalToWorld_Point(center - halfSize);
	Debug::CreateWireframe_Temp(w, Vector(), Vector(0.125f, 0.125f, 0.125f));
	return w;
}
Vector BoxCollider::GetWorldMax() const
{
	Vector w = GetTransform()->LocalToWorld_Point(center + halfSize);
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