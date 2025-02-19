#pragma once

#include "Matrix.h"

#include <DirectXMath.h>

class Transform
{
private:
	Vector location;
	Vector rotation;
	Vector scale;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	// Flag for if the stored world matrix needs to be updated to reflect transformations
	bool isWorldMatrixDirty;

public:
	Transform();
	Transform(Vector location, Vector rotation, Vector scale);

	void MoveAbsolute(Vector vector);
	void MoveRelative(Vector vector);
	void Rotate(Vector pitchYawRoll);
	void Scale(Vector vector);

	Vector GetLocation() const;
	Vector GetPitchYawRoll() const;
	Vector GetScale() const;
	Vector GetRight() const;
	Vector GetUp() const;
	Vector GetForward() const;
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	const Vector* GetLocationPtr() const { return &location; };
	const Vector* GetPitchYawRollPtr() const { return &rotation; };
	const Vector* GetScalePtr() const { return &scale; };

	void SetLocation(Vector location);
	void SetRotation(Vector rotation);
	void SetScale(Vector scale);

	Vector LocalToWorld(const Vector& vector, bool includeScale = false);
	Vector WorldToLocal_Point(const Vector& vector);
	Vector WorldToLocal_Direction(const Vector& vector);
};