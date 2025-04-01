#include "Matrix.h"

#include <DirectXMath.h>

Matrix4x4::Matrix4x4()
{

}
Matrix4x4::Matrix4x4(DirectX::XMFLOAT4X4 matrix)
{
    for(int i = 0; i < 4; i++)
        rows[i] = Vector(matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
}
Matrix4x4::Matrix4x4(const Vector& row0, const Vector& row1, const Vector& row2, const Vector& row3)
{
	rows[0] = row0;
	rows[1] = row1;
	rows[2] = row2;
	rows[3] = row3;
}
Matrix4x4::~Matrix4x4()
{

}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
	DirectX::XMFLOAT4X4 f44_this = *this;	
	DirectX::XMFLOAT4X4 f44_other = other;

	DirectX::XMFLOAT4X4 f44_result;
	DirectX::XMStoreFloat4x4(&f44_result, DirectX::XMMatrixMultiply(
		DirectX::XMLoadFloat4x4(&f44_this),
		DirectX::XMLoadFloat4x4(&f44_other)));

	return f44_result;
}
Matrix4x4 Matrix4x4::operator*(float scalar) const
{
	return Matrix4x4(rows[0] * scalar, rows[1] * scalar, rows[2] * scalar, rows[3] * scalar);
}

Vector Matrix4x4::operator*(const Vector& vector) const
{
    return Vector(rows[0].Dot(vector), rows[1].Dot(vector), rows[2].Dot(vector), rows[3].Dot(vector));
}

Matrix4x4::operator DirectX::XMFLOAT4X4() const
{
	return DirectX::XMFLOAT4X4(
		rows[0].x, rows[0].y, rows[0].z, rows[0].w,
		rows[1].x, rows[1].y, rows[1].z, rows[1].w,
		rows[2].x, rows[2].y, rows[2].z, rows[2].w,
		rows[3].x, rows[3].y, rows[3].z, rows[3].w);
}

Matrix4x4 Matrix4x4::Transpose() const
{
	return Matrix4x4(
		Vector(rows[0].x, rows[1].x, rows[2].x, rows[3].x),
		Vector(rows[0].y, rows[1].y, rows[2].y, rows[3].y),
		Vector(rows[0].z, rows[1].z, rows[2].z, rows[3].z),
		Vector(rows[0].w, rows[1].w, rows[2].w, rows[3].w));
}
Matrix4x4 Matrix4x4::Inverse() const
{
	DirectX::XMMATRIX xmm_inverse = DirectX::XMMatrixInverse(nullptr, 
		DirectX::XMMATRIX(
			rows[0].x, rows[0].y, rows[0].z, rows[0].w,
			rows[1].x, rows[1].y, rows[1].z, rows[1].w,
			rows[2].x, rows[2].y, rows[2].z, rows[2].w,
			rows[3].x, rows[3].y, rows[3].z, rows[3].w));

	DirectX::XMFLOAT4X4 f44_result;
	DirectX::XMStoreFloat4x4(&f44_result, xmm_inverse);

	return f44_result;
}

Matrix4x4 Matrix4x4::Scale(const Vector& scale)
{
	Matrix4x4 result = Identity();
	result.rows[0].x = scale.x;
	result.rows[1].y = scale.y;
	result.rows[2].z = scale.z;
	result.rows[3].w = scale.w;

	return result;
}

Matrix4x4 Matrix4x4::Transform(const Vector& translation, const Vector& rotation, const Vector& scale)
{
	DirectX::XMFLOAT3 f3_location(translation.x, translation.y, translation.z);
	DirectX::XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);
	DirectX::XMFLOAT3 f3_scale(scale.x, scale.y, scale.z);

	DirectX::XMVECTOR xmv_translation = XMLoadFloat3(&f3_location);
	DirectX::XMVECTOR xmv_rotation = XMLoadFloat4(&f4_rotation);
	DirectX::XMVECTOR xmv_scale = XMLoadFloat3(&f3_scale);

	DirectX::XMMATRIX xmm_translation = DirectX::XMMatrixTranslationFromVector(xmv_translation);
	DirectX::XMMATRIX xmm_rotation = DirectX::XMMatrixRotationQuaternion(xmv_rotation);
	DirectX::XMMATRIX xmm_scale = DirectX::XMMatrixScalingFromVector(xmv_scale);

	DirectX::XMMATRIX xmm_world = xmm_scale * xmm_rotation * xmm_translation;

	DirectX::XMFLOAT4X4 f44_result;
	XMStoreFloat4x4(&f44_result, xmm_world);

	return f44_result;
}