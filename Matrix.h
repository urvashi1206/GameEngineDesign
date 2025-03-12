#pragma once

#include "Vector.h"

#include <DirectXMath.h>

struct Matrix4x4
{
private:
	Vector rows[4];

public:
	Matrix4x4();
	Matrix4x4(DirectX::XMFLOAT4X4 matrix);
	Matrix4x4(const Vector& row0 = Vector(1, 0, 0, 0), const Vector& row1 = Vector(0, 1, 0, 0), const Vector& row2 = Vector(0, 0, 1, 0), const Vector& row3 = Vector(0, 0, 0, 1));
	~Matrix4x4();

	Matrix4x4 operator*(const Matrix4x4& other) const;
	Matrix4x4 operator*(float scalar) const;

	Vector operator*(const Vector& vector) const;

	Matrix4x4 Transpose() const;
	Matrix4x4 Inverse() const;

	DirectX::XMFLOAT4X4 ToXMFLOAT4X4() const;

	static Matrix4x4 Identity()
	{
		return Matrix4x4(
			Vector(1, 0, 0, 0),
			Vector(0, 1, 0, 0),
			Vector(0, 0, 1, 0),
			Vector(0, 0, 0, 1));
	};

	static Matrix4x4 Scale(const Vector& scale);
};