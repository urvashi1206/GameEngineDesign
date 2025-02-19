#pragma once

#include "Vector.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

struct DebugMesh
{
	//Transform* transform;
	const Vector* location;
	const Vector* rotation;
	const Vector* scale;
	std::shared_ptr<Mesh> mesh;

	DebugMesh(Transform* transform, std::shared_ptr<Mesh> mesh) : 
		location(transform->GetLocationPtr()), rotation(transform->GetPitchYawRollPtr()), scale(transform->GetScalePtr()), mesh(mesh)
	{
		
	}

	virtual Transform GetTransform() { return Transform(
		DirectX::XMFLOAT3(location->x, location->y, location->z),
		DirectX::XMFLOAT3(rotation->x, rotation->y, rotation->z),
		DirectX::XMFLOAT3(scale->x, scale->y, scale->z)); };
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
	DebugBox(std::shared_ptr<Mesh> mesh, Vector location, Vector rotation, Vector halfSize) : DebugMesh(nullptr, mesh),
		offset(location), rotationOffset(rotation), scaleOffset(halfSize), halfSize(halfSize)
	{

	}

	Transform GetTransform() override
	{
		if(location && rotation && scale)
		{
			Transform transform = DebugMesh::GetTransform();
			transform.SetScale(halfSize);

			return transform;
		}
		else return Transform(offset, rotationOffset, scaleOffset);
	};
};

namespace Debug
{
	void CreateDebugBox(Transform* transform, Vector offset, Vector halfSize);
	void CreateDebugBox(Vector* location, Vector* rotation, Vector* scale);

	void CreateWireframe_Temp(Vector location, Vector rotation = Vector(0, 0, 0), Vector halfSize = Vector(0.5f, 0.5f, 0.5f));

	void DrawAllWireframes(std::shared_ptr<Camera> camera);

	void SetDebugMaterial(std::shared_ptr<Material> material);
	void SetDebugBoxMesh(std::shared_ptr<Mesh> mesh);
}