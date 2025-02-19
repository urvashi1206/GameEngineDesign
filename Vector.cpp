#include "Vector.h"

#include <math.h>

Vector::Vector(float x, float y, float z, float w) : 
	x(x), y(y), z(z), w(w)
{
	
}
Vector::Vector(DirectX::XMFLOAT3 vector) : Vector(vector.x, vector.y, vector.z)
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

Vector Vector::Normalized() const
{
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