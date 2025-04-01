#include "Transform.h"

#include "Constants.h"

using namespace DirectX;

Transform::Transform() : 
	location(0, 0, 0),
	rotation(0, 0, 0, 1),
	scale(1, 1, 1),
	isWorldMatrixDirty(false)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}
Transform::Transform(Vector location, Vector rotation, Vector scale) : 
	location(location), 
	rotation(0, 0, 0, 1),
	scale(scale),
	isWorldMatrixDirty(true) // Matrices aren't calculated yet
{
	if(rotation.w == 0)
		SetRotationPitchYawRoll(rotation);
	else
		SetRotation(rotation);

	// Default matrices; will be calculated the first time GetWorldMatrix() or GetWorldInverseTransposeMatrix() are called
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}
Transform::~Transform()
{
	
}

void Transform::MoveAbsolute(Vector vector)
{
	SetLocation(location + vector);
}
void Transform::MoveRelative(Vector vector)
{
	XMFLOAT3 currentLocation(location.x, location.y, location.z);
	XMFLOAT3 offset(vector.x, vector.y, vector.z);
	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);

	// Get the absolute offset by rotating relative offset by this transform's rotation
	XMVECTOR relOffset = XMLoadFloat3(&offset);
	XMVECTOR absOffset = XMVector3Rotate(relOffset, XMLoadFloat4(&f4_rotation));

	XMVECTOR currentLocationVector = XMLoadFloat3(&currentLocation);

	// Add absolute offset to current location
	XMFLOAT3 destination;
	XMStoreFloat3(&destination, currentLocationVector + absOffset);
	SetLocation(destination);
}
void Transform::Rotate(Vector quaternion)
{
	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);
	XMFLOAT4 f4_delta(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

	XMVECTOR xmv_result = XMQuaternionMultiply(XMLoadFloat4(&f4_rotation), XMLoadFloat4(&f4_delta));
	
	XMFLOAT4 f4_result;
	XMStoreFloat4(&f4_result, xmv_result);

	SetRotation(f4_result);
}
void Transform::RotateAxisAngle(Vector axisAngle)
{
	if(axisAngle == 0)
		return;

	RotateAxisAngle(axisAngle.Normalized(), axisAngle.GetMagnitude());
}
void Transform::RotateAxisAngle(Vector axis, float angle)
{
	if(axis == 0)
		return;

	DirectX::XMFLOAT3 f3_rotationAxis(axis.x, axis.y, axis.z);
	DirectX::XMVECTOR xmv_rot = DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&f3_rotationAxis), angle);
	
	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);
	XMVECTOR xmv_rotation = DirectX::XMQuaternionMultiply(XMLoadFloat4(&f4_rotation), xmv_rot);

	XMStoreFloat4(&f4_rotation, xmv_rotation);

	SetRotation(f4_rotation);
}
void Transform::RotatePitchYawRoll(Vector pitchYawRoll)
{
	SetRotationPitchYawRoll(GetPitchYawRoll() + pitchYawRoll);

	/*XMFLOAT4 f4_rot;
	XMStoreFloat4(&f4_rot, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));
	
	Rotate(f4_rot);*/
}
void Transform::Scale(Vector vector)
{
	SetScale(scale * vector);
}

void Transform::Initialize()
{

}
void Transform::Update(float deltaTime)
{

}

#pragma region Getters
Vector Transform::GetLocation() const
{
	return location;
}
Vector Transform::GetRotation() const
{
	return rotation;
}
Vector Transform::GetPitchYawRoll() const
{
	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);

	float x = f4_rotation.x;
	float y = f4_rotation.y;
	float z = f4_rotation.z;
	float w = f4_rotation.w;

	// Compute Yaw (rotation around Y-axis)
	float yaw = atan2f(2.0f * (w * y + x * z), 1.0f - 2.0f * (y * y + z * z));

	// Compute Pitch (rotation around X-axis)
	float sinp = 2.0f * (w * x - y * z);
	float pitch = fabs(sinp) >= 1.0f ? copysignf(DirectX::XM_PIDIV2, sinp) : asinf(sinp);

	// Compute Roll (rotation around Z-axis)
	float roll = atan2f(2.0f * (w * z + x * y), 1.0f - 2.0f * (x * x + y * y));

	return Vector(pitch, yaw, roll); // Returned in radians
}
Vector Transform::GetScale() const { return scale; }
Vector Transform::GetRight() const
{
	// RotatePitchYawRoll the world right vector by this transform's rotation
	XMFLOAT3 worldRight = XMFLOAT3(1, 0, 0);

	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);

	XMFLOAT3 right;
	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&worldRight), XMLoadFloat4(&f4_rotation)));
	return right;
}
Vector Transform::GetUp() const
{
	// RotatePitchYawRoll the world up vector by this transform's rotation
	XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);

	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);

	XMFLOAT3 up;
	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&worldUp), XMLoadFloat4(&f4_rotation)));
	return up;
}
Vector Transform::GetForward() const
{
	// RotatePitchYawRoll the world forward vector by this transform's rotation
	XMFLOAT3 worldForward = XMFLOAT3(0, 0, 1);

	XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);

	XMFLOAT3 forward;
	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&worldForward), XMLoadFloat4(&f4_rotation)));
	return forward;
}
XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if(isWorldMatrixDirty)
	{
		XMFLOAT3 f3_location(location.x, location.y, location.z);
		XMFLOAT4 f4_rotation(rotation.x, rotation.y, rotation.z, rotation.w);
		XMFLOAT3 f3_scale(scale.x, scale.y, scale.z);

		XMVECTOR xmv_translation = XMLoadFloat3(&f3_location);
		XMVECTOR xmv_rotation = XMLoadFloat4(&f4_rotation);
		XMVECTOR xmv_scale = XMLoadFloat3(&f3_scale);

		XMMATRIX xmm_translation = XMMatrixTranslationFromVector(xmv_translation);
		XMMATRIX xmm_rotation = XMMatrixRotationQuaternion(xmv_rotation);
		XMMATRIX xmm_scale = XMMatrixScalingFromVector(xmv_scale);

		XMMATRIX xmm_world = xmm_scale * xmm_rotation * xmm_translation;

		XMStoreFloat4x4(&worldMatrix, xmm_world);
		XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(xmm_world)));

		isWorldMatrixDirty = false;
	}

	return worldMatrix;
}
XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	// Make sure the matrices are up-to-date
	if(isWorldMatrixDirty)
		GetWorldMatrix();

	return worldInverseTransposeMatrix;
}

Matrix4x4 Transform::GetRotationMatrix() const
{
	return Matrix4x4(GetRight(), GetUp(), GetForward());
}
#pragma endregion

#pragma region Setters
void Transform::SetLocation(Vector location)
{
	this->location = location;

	isWorldMatrixDirty = true;
}
void Transform::SetRotation(Vector quaternion)
{
	this->rotation = quaternion;

	isWorldMatrixDirty = true;
}
void Transform::SetRotationPitchYawRoll(Vector pitchYawRoll)
{
	XMFLOAT4 f4_rotation;
	XMStoreFloat4(&f4_rotation, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x * DEG2RAD, pitchYawRoll.y * DEG2RAD, pitchYawRoll.z * DEG2RAD));

	//XMVECTOR xmv_pitch = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), pitchYawRoll.x);
	//XMVECTOR xmv_yaw = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), pitchYawRoll.y);
	//XMVECTOR xmv_roll = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), pitchYawRoll.z);

	////XMFLOAT4 f4_rotation;
	//XMStoreFloat4(&f4_rotation, XMQuaternionMultiply(XMQuaternionMultiply(xmv_pitch, xmv_yaw), xmv_roll));

	SetRotation(f4_rotation);
}
void Transform::SetScale(Vector scale)
{
	this->scale = scale;

	isWorldMatrixDirty = true;
}
#pragma endregion

Vector Transform::LocalToWorld_Point(const Vector& vector, bool includeScale)
{
	return GetLocation() + GetRight() * vector.x + GetUp() * vector.y + GetForward() * vector.z;
}
Vector Transform::LocalToWorld_Direction(const Vector& vector, bool includeScale)
{
	return GetRight() * vector.x + GetUp() * vector.y + GetForward() * vector.z;
}
Vector Transform::WorldToLocal_Point(const Vector& vector)
{
	Vector centered = vector - GetLocation();

	return Vector(
		centered.ProjectOnto(GetRight()).GetMagnitude(),
		centered.ProjectOnto(GetUp()).GetMagnitude(),
		centered.ProjectOnto(GetForward()).GetMagnitude());
}
Vector Transform::WorldToLocal_Direction(const Vector& vector)
{
	XMFLOAT3 worldDirection(vector.x, vector.y, vector.z);
	XMVECTOR worldDir = XMLoadFloat3(&worldDirection);

	//XMVECTOR localDirectionVector = XMVector3Rotate(worldDir, XMQuaternionInverse(XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
	XMVECTOR localDirectionVector = XMVector3Rotate(worldDir, XMQuaternionRotationRollPitchYaw(-rotation.x * DEG2RAD, -rotation.y * DEG2RAD, -rotation.z * DEG2RAD));
	
	XMFLOAT3 localDirection;
	XMStoreFloat3(&localDirection, localDirectionVector);

	/*XMFLOAT4X4 worldMatrix = GetWorldMatrix();
	worldMatrix.
	worldMatrix.r[3] = XMVectorSet(0, 0, 0, 1);*/

	return Vector(localDirection);
}