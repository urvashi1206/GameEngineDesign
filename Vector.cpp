#include "Vector.h"
#include "IntVector.h"

#include <math.h>

Vector::Vector(float x, float y, float z, float w) : 
	x(x), y(y), z(z), w(w)
{
	
}
Vector::Vector(IntVector vector) : x(vector.x), y(vector.y), z(vector.z), w(vector.w)
{
	
}
Vector::Vector(DirectX::XMFLOAT3 vector) : Vector(vector.x, vector.y, vector.z)
{

}
Vector::Vector(DirectX::XMFLOAT4 vector) : Vector(vector.x, vector.y, vector.z, vector.w)
{

}
Vector::~Vector()
{

}

Vector Vector::operator+(const Vector& other) const
{
	return Vector(x + other.x, y + other.y, z + other.z, w + other.w);
}
Vector Vector::operator-(const Vector& other) const
{
	return Vector(x - other.x, y - other.y, z - other.z, w - other.w);
}
Vector Vector::operator*(float scalar) const
{
	return Vector(x * scalar, y * scalar, z * scalar, w * scalar);
}
Vector Vector::operator*(const Vector& other) const
{
	return Vector(x * other.x, y * other.y, z * other.z, w * other.w);
}
Vector Vector::operator/(float scalar) const
{
	return Vector(x / scalar, y / scalar, z / scalar, w / scalar);
}
Vector Vector::operator/(const Vector& other) const
{
	return Vector(x / other.x, y / other.y, z / other.z, w / other.w);
}

void Vector::operator+=(const Vector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
}
void Vector::operator-=(const Vector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
}
void Vector::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
}
void Vector::operator*=(const Vector& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;
}
void Vector::operator/=(float scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
}
void Vector::operator/=(const Vector& other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	w /= other.w;
}

bool Vector::operator==(const Vector& other) const
{
	return x == other.x && y == other.y && z == other.z && w == other.w;
}

const Vector Vector::operator-() const
{
	return Vector(-x, -y, -z, -w);
}

float Vector::Dot(const Vector& other) const
{
	return x * other.x + y * other.y + z * other.z + w * other.w;
}
Vector Vector::Cross(const Vector& other) const
{
	/* Credit: https://github.com/simenandresen/Simulink-Underwater-Robotics-Simulator/blob/master/3rdparty/gnc_mfiles/Smtrx.m */

	return Vector(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x);
}

Vector Vector::ProjectOnto(const Vector& other) const
{
	return other.Normalized() * (this->Dot(other) / other.Dot(other));
}
Vector Vector::ProjectOntoPlane(const Vector& normal) const
{
	// If vector is orthogonal to plane, return zero
	if(!this->IsLinearlyIndependent(normal))
		return Vector();

	Vector v = this->Cross(normal);
	Vector u = normal.Cross(v);

	return this->ProjectOnto(v) + this->ProjectOnto(u);
}
Vector Vector::ProjectOntoPlane(const Vector& point, const Vector& normal) const
{
	Vector dir = (*this - point).ProjectOntoPlane(normal);

	return point + dir;
}

Vector Vector::Reflect(const Vector& axis, const Vector& origin) const
{
	return *this + (origin - *this).ProjectOntoPlane(axis) * 2;
}

Vector Vector::Normalized() const
{
	if(*this == Vector::Zero())
		return Vector::Zero();

	return Vector(x, y, z, w) / GetMagnitude();
}

float Vector::GetMagnitude() const
{
	return sqrt(GetSqrMagnitude());
}
float Vector::GetSqrMagnitude() const
{
	return x*x + y*y + z*z + w*w;
}
float Vector::Distance(const Vector& other) const
{
	return (other - *this).GetMagnitude();
}

bool Vector::IsLinearlyIndependent(const Vector& other) const
{
	return !(this->Normalized() == other.Normalized() || this->Normalized() == -other.Normalized());
}

void Vector::GetOrthogonalPlane(Vector& out_u, Vector& out_v)
{
	// Arbitrary independent reference vector
	Vector referenceVector(1, 0, 0);
	if(!this->IsLinearlyIndependent(referenceVector))
		referenceVector = Vector(0, 1, 0);

	out_u = this->Cross(referenceVector);
	out_v = this->Cross(out_u);
}

Vector Vector::Ceil() const
{
	return Vector(ceilf(x), ceilf(y), ceilf(z), ceilf(w));
}

Vector Vector::Lerp(const Vector& a, const Vector& b, float t)
{
	return a * (1 - t) + b * t;
}