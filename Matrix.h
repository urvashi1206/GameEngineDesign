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
	~Matrix4x4();

	const Vector& operator*(const Vector& vector) const;
};