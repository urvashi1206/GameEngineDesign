#include "IntVector.h"

IntVector::IntVector(int x, int y, int z, int w) : 
    x(x), y(y), z(z), w(w)
{

}
IntVector::IntVector(Vector vector) : IntVector(vector.x, vector.y, vector.z, vector.w)
{

}
IntVector::~IntVector()
{

}

IntVector IntVector::operator+(const IntVector& other) const
{
    return IntVector(x + other.x, y + other.y, z + other.z, w + other.w);
}
IntVector IntVector::operator-(const IntVector& other) const
{
    return IntVector(x - other.x, y - other.y, z - other.z, w - other.w);
}
IntVector IntVector::operator*(int scalar) const
{
    return IntVector(x * scalar, y * scalar, z * scalar, w * scalar);
}
IntVector IntVector::operator*(const IntVector& other) const
{
    return IntVector(x * other.x, y * other.y, z * other.z, w * other.w);
}
IntVector IntVector::operator/(int scalar) const
{
    return IntVector(x / scalar, y / scalar, z / scalar, w / scalar);
}
IntVector IntVector::operator/(const IntVector& other) const
{
    return IntVector(x / other.x, y / other.y, z / other.z, w / other.w);
}
IntVector IntVector::operator%(int scalar) const
{
    return IntVector(x % scalar, y % scalar, z % scalar, w % scalar);
}

Vector IntVector::operator/(float scalar) const
{
    return Vector(x / scalar, y / scalar, z / scalar, w / scalar);
}
Vector IntVector::operator/(const Vector& other) const
{
    return Vector(x / other.x, y / other.y, z / other.z, w / other.w);
}

void IntVector::operator+=(const IntVector& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
}
void IntVector::operator-=(const IntVector& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
}
void IntVector::operator*=(int scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
}
void IntVector::operator*=(const IntVector& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;
}
void IntVector::operator/=(int scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
}
void IntVector::operator/=(const IntVector& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;
}

bool IntVector::operator==(const IntVector& other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}
bool IntVector::operator<=(const IntVector& other) const
{
    return x <= other.x && y <= other.y && z <= other.z && w <= other.w;
}
bool IntVector::operator>=(const IntVector& other) const
{
    return x >= other.x && y >= other.y && z >= other.z && w >= other.w;
}

const IntVector IntVector::operator-() const
{
    return IntVector(-x, -y, -z, -w);
}

IntVector IntVector::Abs() const
{
    return IntVector(abs(x), abs(y), abs(z), abs(w));
}

IntVector IntVector::Rand(const IntVector& minInclusive, const IntVector& maxExclusive)
{
    return Vector(
        minInclusive.x + rand() % (maxExclusive.x - minInclusive.x),
        minInclusive.y + rand() % (maxExclusive.y - minInclusive.y),
        minInclusive.z + rand() % (maxExclusive.z - minInclusive.z));
}