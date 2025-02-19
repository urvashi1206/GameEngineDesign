#pragma once

#include <DirectXMath.h>

struct Vector
{
public:
	float x, y, z, w;

	Vector(float x = 0, float y = 0, float z = 0, float w = 0);
	Vector(DirectX::XMFLOAT3 vector);
	~Vector();

	Vector operator+(const Vector& other) const;
	Vector operator-(const Vector& other) const;
	Vector operator*(float scalar) const;
	Vector operator*(const Vector& other) const;
	Vector operator/(float scalar) const;
	Vector operator/(const Vector& other) const;

	void operator+=(const Vector& other);
	void operator-=(const Vector& other);
	void operator*=(float scalar);
	void operator*=(const Vector& other);
	void operator/=(float scalar);
	void operator/=(const Vector& other);

	bool operator==(const Vector& other) const;

	const Vector operator-() const;

	float Dot(const Vector& other) const;
	Vector Cross(const Vector& other) const;

	Vector ProjectOnto(const Vector& other) const;

	Vector Normalized() const;

	float GetMagnitude() const;
	float GetSqrMagnitude() const;
};