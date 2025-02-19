#include "Transform.h"

#include "Constants.h"

using namespace DirectX;

Transform::Transform() : 
	location(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1),
	isWorldMatrixDirty(false)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}
Transform::Transform(Vector location, Vector rotation, Vector scale) : 
	location(location), 
	rotation(rotation), 
	scale(scale),
	isWorldMatrixDirty(true) // Matrices aren't calculated yet
{
	// Default matrices; will be calculated the first time GetWorldMatrix() or GetWorldInverseTransposeMatrix() are called
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}

void Transform::MoveAbsolute(Vector vector)
{
	SetLocation(location + vector);
}
void Transform::MoveRelative(Vector vector)
{
	XMFLOAT3 currentLocation(location.x, location.y, location.z);
	XMFLOAT3 offset = XMFLOAT3(vector.x, vector.y, vector.z);

	// Get the absolute offset by rotating relative offset by this transform's rotation
	XMVECTOR relOffset = XMLoadFloat3(&offset);
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD);
	XMVECTOR absOffset = XMVector3Rotate(relOffset, rot);

	XMVECTOR currentLocationVector = XMLoadFloat3(&currentLocation);

	// Add absolute offset to current location
	XMFLOAT3 destination;
	XMStoreFloat3(&destination, currentLocationVector + absOffset);
	SetLocation(destination);
}
void Transform::Rotate(Vector pitchYawRoll)
{
	SetRotation(rotation + pitchYawRoll);
}
void Transform::Scale(Vector vector)
{
	SetScale(scale * vector);
}

#pragma region Getters
Vector Transform::GetLocation() const { return Vector(location); }
Vector Transform::GetPitchYawRoll() const { return Vector(rotation); }
Vector Transform::GetScale() const { return Vector(scale); }
Vector Transform::GetRight() const
{
	// Rotate the world right vector by this transform's rotation
	XMFLOAT3 worldRight = XMFLOAT3(1, 0, 0);

	XMFLOAT3 right;
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD);

	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&worldRight), rot));
	return Vector(right);
}
Vector Transform::GetUp() const
{
	// Rotate the world up vector by this transform's rotation
	XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);

	XMFLOAT3 up;
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD);

	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&worldUp), rot));
	return Vector(up);
}
Vector Transform::GetForward() const
{
	// Rotate the world forward vector by this transform's rotation
	XMFLOAT3 worldForward = XMFLOAT3(0, 0, 1);

	XMFLOAT3 forward;
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD);

	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&worldForward), rot));
	return Vector(forward);
}
XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if(isWorldMatrixDirty)
	{
		XMFLOAT3 locationFloat3(location.x, location.y, location.z);
		XMFLOAT3 rotationFloat3(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD);
		XMFLOAT3 scaleFloat3(scale.x, scale.y, scale.z);

		XMVECTOR translationVector = XMLoadFloat3(&locationFloat3);
		XMVECTOR rotationVector = XMLoadFloat3(&rotationFloat3);
		XMVECTOR scaleVector = XMLoadFloat3(&scaleFloat3);

		XMMATRIX translation = XMMatrixTranslationFromVector(translationVector);
		XMMATRIX rotation = XMMatrixRotationRollPitchYawFromVector(rotationVector);
		XMMATRIX scale = XMMatrixScalingFromVector(scaleVector);

		XMMATRIX world = scale * rotation * translation;

		XMStoreFloat4x4(&worldMatrix, world);
		XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

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
#pragma endregion

#pragma region Setters
void Transform::SetLocation(Vector location)
{
	this->location = location;

	isWorldMatrixDirty = true;
}
void Transform::SetRotation(Vector rotation)
{
	this->rotation = rotation;

	isWorldMatrixDirty = true;
}
void Transform::SetScale(Vector scale)
{
	this->scale = scale;

	isWorldMatrixDirty = true;
}
#pragma endregion

Vector Transform::LocalToWorld(const Vector& vector, bool includeScale)
{
	return GetLocation() + GetRight() * vector.x + GetUp() * vector.y + GetForward() * vector.z;
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

	return Vector(
		vector.ProjectOnto(GetRight()).GetMagnitude(),
		vector.ProjectOnto(GetUp()).GetMagnitude(),
		vector.ProjectOnto(GetForward()).GetMagnitude());
}