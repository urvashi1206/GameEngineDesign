#pragma once

#include "Vector.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

struct DebugMesh
{
	Vector location;
	Vector rotation;
	Vector scale;
	std::shared_ptr<Mesh> mesh;

	DebugMesh(Transform* transform, std::shared_ptr<Mesh> mesh) : 
		location(transform->GetLocation()), rotation(transform->GetPitchYawRoll()), scale(transform->GetScale()), mesh(mesh)
	{
		
	}
	DebugMesh(Vector location, Vector rotation, Vector scale, std::shared_ptr<Mesh> mesh) :
		location(location), rotation(rotation), scale(scale), mesh(mesh)
	{

	}

	virtual Matrix4x4 GetWorldMatrix() { return Matrix4x4::Transform(location, rotation, scale); };
};

struct DebugBox : DebugMesh
{
	Vector offset;
	Vector rotationOffset;
	Vector scaleOffset;
	Vector halfSize;

	DebugBox(Transform* transform, std::shared_ptr<Mesh> mesh, Vector offset, Vector halfSize) : DebugMesh(transform, mesh), 
		offset(offset), halfSize(halfSize)
	{

	}
	DebugBox(std::shared_ptr<Mesh> mesh, Vector location, Vector rotation, Vector halfSize) : DebugMesh(location, rotation, halfSize, mesh),
		offset(location), rotationOffset(rotation), scaleOffset(halfSize), halfSize(halfSize)
	{

	}

	Matrix4x4 GetWorldMatrix() override
	{
		return DebugMesh::GetWorldMatrix();
	};
};
struct DebugSphere : DebugMesh
{
	Vector offset;
	Vector rotationOffset;
	Vector scaleOffset;
	Vector radius;

	DebugSphere(Transform* transform, std::shared_ptr<Mesh> mesh, Vector offset, float radius) : DebugMesh(transform, mesh),
		offset(offset), radius(radius)
	{

	}
	DebugSphere(std::shared_ptr<Mesh> mesh, Vector location, Vector rotation, float radius) : DebugMesh(nullptr, mesh),
		offset(location), rotationOffset(rotation), scaleOffset(radius), radius(radius)
	{

	}

	Matrix4x4 GetWorldMatrix() override
	{
		return DebugMesh::GetWorldMatrix();
	};
};

namespace Debug
{
	void CreateDebugBox(Transform* transform, Vector offset, Vector halfSize);
	void CreateDebugBox(Vector* location, Vector* rotation, Vector* scale);
	void CreateDebugSphere(Transform* transform, Vector offset, float radius);

	void CreateWireframe_Temp(Vector location, Vector rotation = Vector(0, 0, 0), Vector halfSize = Vector(0.5f, 0.5f, 0.5f));
	void CreateWireframeSphere_Temp(Vector location, Vector rotation = Vector(0, 0, 0), float radius = 0.5f);

	void DrawAllWireframes(std::shared_ptr<Camera> camera);

	void SetDebugMaterial(std::shared_ptr<Material> material);
	void SetDebugBoxMesh(std::shared_ptr<Mesh> mesh);
	void SetDebugSphereMesh(std::shared_ptr<Mesh> mesh);
}