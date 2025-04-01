#pragma once

#include "Component.h"

#include "Matrix.h"

#include <DirectXMath.h>

class Transform : public Component
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
	~Transform();

	void MoveAbsolute(Vector vector);
	void MoveRelative(Vector vector);
	void Rotate(Vector quaternion);
	void RotateAxisAngle(Vector axisAngle);
	void RotateAxisAngle(Vector axis, float angle);
	void RotatePitchYawRoll(Vector pitchYawRoll);
	void Scale(Vector vector);

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

public:
	Vector GetLocation() const;
	Vector GetRotation() const;
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

	Matrix4x4 GetRotationMatrix() const;

	void SetLocation(Vector location);
	void SetRotation(Vector quaternion);
	void SetRotationPitchYawRoll(Vector pitchYawRoll);
	void SetScale(Vector scale);

	Vector LocalToWorld_Point(const Vector& vector, bool includeScale = false);
	Vector LocalToWorld_Direction(const Vector& vector, bool includeScale = false);
	Vector WorldToLocal_Point(const Vector& vector);
	Vector WorldToLocal_Direction(const Vector& vector);
};