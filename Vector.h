#pragma once

#include <algorithm>

#include <DirectXMath.h>

struct Vector
{
public:
	float x, y, z, w;

	Vector(float x = 0, float y = 0, float z = 0, float w = 0);
	Vector(DirectX::XMFLOAT3 vector);
	Vector(DirectX::XMFLOAT4 vector);
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
	Vector ProjectOntoPlane(const Vector& normal) const;
	Vector ProjectOntoPlane(const Vector& point, const Vector& normal) const;

	Vector Normalized() const;

	float GetMagnitude() const;
	float GetSqrMagnitude() const;
	float Distance(const Vector& other) const;

	bool IsLinearlyIndependent(const Vector& other) const;

	void GetOrthogonalPlane(Vector& out_u, Vector& out_v);

	static Vector Lerp(const Vector& a, const Vector& b, float t);
};

namespace std
{
	template<>
	struct hash<Vector>
	{
		std::size_t operator()(const Vector& vector) const
		{
			/* Hash and combine x, y, z, w */

			std::hash<float> hasher;

			return hasher(vector.x) 
				^ (hasher(vector.y) << 1) 
				^ (hasher(vector.z) << 2) 
				^ (hasher(vector.w) << 3);
		}
	};
}