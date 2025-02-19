#include "Matrix.h"

Matrix4x4::Matrix4x4()
{

}
Matrix4x4::Matrix4x4(DirectX::XMFLOAT4X4 matrix)
{
    for(int i = 0; i < 4; i++)
        rows[i] = Vector(matrix.m[0][i], matrix.m[1][i], matrix.m[2][i], matrix.m[3][i]);
}
Matrix4x4::~Matrix4x4()
{

}

const Vector& Matrix4x4::operator*(const Vector& vector) const
{
    return Vector(rows[0].Dot(vector), rows[1].Dot(vector), rows[2].Dot(vector), rows[3].Dot(vector));
}