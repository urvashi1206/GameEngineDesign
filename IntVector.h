#pragma once

#include "Vector.h"

struct IntVector
{
public:
	int x, y, z, w;

	IntVector(int x = 0, int y = 0, int z = 0, int w = 0);
	IntVector(Vector vector);
	~IntVector();

	IntVector operator+(const IntVector& other) const;
	IntVector operator-(const IntVector& other) const;
	IntVector operator*(int scalar) const;
	IntVector operator*(const IntVector& other) const;
	IntVector operator/(int scalar) const;
	IntVector operator/(const IntVector& other) const;
	IntVector operator%(int scalar) const;

	Vector operator/(float scalar) const;
	Vector operator/(const Vector& other) const;

	void operator+=(const IntVector& other);
	void operator-=(const IntVector& other);
	void operator*=(int scalar);
	void operator*=(const IntVector& other);
	void operator/=(int scalar);
	void operator/=(const IntVector& other);

	bool operator==(const IntVector& other) const;
	bool operator<=(const IntVector& other) const;
	bool operator>=(const IntVector& other) const;

	const IntVector operator-() const;

	IntVector Abs() const;

	static IntVector Zero() { return IntVector(0, 0, 0); };
	static IntVector One() { return IntVector(1, 1, 1); };

	static IntVector Rand(const IntVector& minInclusive, const IntVector& maxExclusive);
};

namespace std
{
	template<>
	struct hash<IntVector>
	{
		std::size_t operator()(const IntVector& vector) const
		{
			/* Hash and combine x, y, z, w */

			std::hash<int> hasher;

			return hasher(vector.x)
				^ (hasher(vector.y) << 1)
				^ (hasher(vector.z) << 2)
				^ (hasher(vector.w) << 3);
		}
	};
}