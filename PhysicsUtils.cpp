#include "PhysicsUtils.h"

#include "Constants.h"

using namespace DirectX;

#pragma region TransformUtils
void TransformUtils::MoveAbsolute(Transform& transform, Vector vector)
{
	SetLocation(transform, transform.location + vector);
}
void TransformUtils::MoveRelative(Transform& transform, Vector vector)
{
	XMFLOAT3 currentLocation(transform.location.x, transform.location.y, transform.location.z);
	XMFLOAT3 offset(vector.x, vector.y, vector.z);
	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

	// Get the absolute offset by rotating relative offset by this transform's rotation
	XMVECTOR relOffset = XMLoadFloat3(&offset);
	XMVECTOR absOffset = XMVector3Rotate(relOffset, XMLoadFloat4(&f4_rotation));

	XMVECTOR currentLocationVector = XMLoadFloat3(&currentLocation);

	// Add absolute offset to current location
	XMFLOAT3 destination;
	XMStoreFloat3(&destination, currentLocationVector + absOffset);
	SetLocation(transform, destination);
}
void TransformUtils::Rotate(Transform& transform, Vector quaternion)
{
	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
	XMFLOAT4 f4_delta(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

	XMVECTOR xmv_result = XMQuaternionMultiply(XMLoadFloat4(&f4_rotation), XMLoadFloat4(&f4_delta));
	
	XMFLOAT4 f4_result;
	XMStoreFloat4(&f4_result, xmv_result);

	SetRotation(transform, f4_result);
}
void TransformUtils::RotateAxisAngle(Transform& transform, Vector axisAngle)
{
	if(axisAngle == 0)
		return;

	RotateAxisAngle(transform, axisAngle.Normalized(), axisAngle.GetMagnitude());
}
void TransformUtils::RotateAxisAngle(Transform& transform, Vector axis, float angle)
{
	if(axis == 0)
		return;

	DirectX::XMFLOAT3 f3_rotationAxis(axis.x, axis.y, axis.z);
	DirectX::XMVECTOR xmv_rot = DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&f3_rotationAxis), angle);
	
	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
	XMVECTOR xmv_rotation = DirectX::XMQuaternionMultiply(XMLoadFloat4(&f4_rotation), xmv_rot);

	XMStoreFloat4(&f4_rotation, xmv_rotation);

	SetRotation(transform, f4_rotation);
}
void TransformUtils::RotatePitchYawRoll(Transform& transform, Vector pitchYawRoll)
{
	SetRotationPitchYawRoll(transform, GetPitchYawRoll(transform) + pitchYawRoll);

	/*XMFLOAT4 f4_rot;
	XMStoreFloat4(&f4_rot, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));
	
	Rotate(f4_rot);*/
}
void TransformUtils::Scale(Transform& transform, Vector vector)
{
	SetScale(transform, transform.scale * vector);
}

#pragma region Getters
Vector TransformUtils::GetPitchYawRoll(const Transform& transform)
{
	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

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
Vector TransformUtils::GetRight(const Transform& transform)
{
	// RotatePitchYawRoll the world right vector by this transform's rotation
	XMFLOAT3 worldRight = XMFLOAT3(1, 0, 0);

	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

	XMFLOAT3 right;
	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&worldRight), XMLoadFloat4(&f4_rotation)));
	return right;
}
Vector TransformUtils::GetUp(const Transform& transform)
{
	// RotatePitchYawRoll the world up vector by this transform's rotation
	XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);

	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

	XMFLOAT3 up;
	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&worldUp), XMLoadFloat4(&f4_rotation)));
	return up;
}
Vector TransformUtils::GetForward(const Transform& transform)
{
	// RotatePitchYawRoll the world forward vector by this transform's rotation
	XMFLOAT3 worldForward = XMFLOAT3(0, 0, 1);

	XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);

	XMFLOAT3 forward;
	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&worldForward), XMLoadFloat4(&f4_rotation)));
	return forward;
}
XMFLOAT4X4 TransformUtils::GetWorldMatrix(Transform& transform)
{
	if(transform.isWorldMatrixDirty)
	{
		XMFLOAT3 f3_location(transform.location.x, transform.location.y, transform.location.z);
		XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
		XMFLOAT3 f3_scale(transform.scale.x, transform.scale.y, transform.scale.z);

		XMVECTOR xmv_translation = XMLoadFloat3(&f3_location);
		XMVECTOR xmv_rotation = XMLoadFloat4(&f4_rotation);
		XMVECTOR xmv_scale = XMLoadFloat3(&f3_scale);

		XMMATRIX xmm_translation = XMMatrixTranslationFromVector(xmv_translation);
		XMMATRIX xmm_rotation = XMMatrixRotationQuaternion(xmv_rotation);
		XMMATRIX xmm_scale = XMMatrixScalingFromVector(xmv_scale);

		XMMATRIX xmm_world = xmm_scale * xmm_rotation * xmm_translation;

		XMStoreFloat4x4(&transform.worldMatrix, xmm_world);
		XMStoreFloat4x4(&transform.worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(xmm_world)));

		transform.isWorldMatrixDirty = false;
	}

	return transform.worldMatrix;
}
XMFLOAT4X4 TransformUtils::GetWorldInverseTransposeMatrix(Transform& transform)
{
	// Make sure the matrices are up-to-date
	if(transform.isWorldMatrixDirty)
		GetWorldMatrix(transform);

	return transform.worldInverseTransposeMatrix;
}

Matrix4x4 TransformUtils::GetRotationMatrix(const Transform& transform)
{
	return Matrix4x4(GetRight(transform), GetUp(transform), GetForward(transform));
}
#pragma endregion

#pragma region Setters
void TransformUtils::SetLocation(Transform& transform, Vector location)
{
	transform.location = location;

	transform.isWorldMatrixDirty = true;
}
void TransformUtils::SetRotation(Transform& transform, Vector quaternion)
{
	transform.rotation = quaternion;

	transform.isWorldMatrixDirty = true;
}
void TransformUtils::SetRotationPitchYawRoll(Transform& transform, Vector pitchYawRoll)
{
	XMFLOAT4 f4_rotation;
	XMStoreFloat4(&f4_rotation, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x * DEG2RAD, pitchYawRoll.y * DEG2RAD, pitchYawRoll.z * DEG2RAD));

	//XMVECTOR xmv_pitch = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), pitchYawRoll.x);
	//XMVECTOR xmv_yaw = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), pitchYawRoll.y);
	//XMVECTOR xmv_roll = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), pitchYawRoll.z);

	////XMFLOAT4 f4_rotation;
	//XMStoreFloat4(&f4_rotation, XMQuaternionMultiply(XMQuaternionMultiply(xmv_pitch, xmv_yaw), xmv_roll));

	SetRotation(transform, f4_rotation);
}
void TransformUtils::SetScale(Transform& transform, Vector scale)
{
	transform.scale = scale;

	transform.isWorldMatrixDirty = true;
}
#pragma endregion

Vector TransformUtils::LocalToWorld_Point(Transform& transform, const Vector& vector, bool includeScale)
{
	return transform.location + GetRight(transform) * vector.x + GetUp(transform) * vector.y + GetForward(transform) * vector.z;
}
Vector TransformUtils::LocalToWorld_Direction(Transform& transform, const Vector& vector, bool includeScale)
{
	return GetRight(transform) * vector.x + GetUp(transform) * vector.y + GetForward(transform) * vector.z;
}
Vector TransformUtils::WorldToLocal_Point(Transform& transform, const Vector& vector)
{
	Vector centered = vector - transform.location;

	return Vector(
		centered.ProjectOnto(GetRight(transform)).GetMagnitude(),
		centered.ProjectOnto(GetUp(transform)).GetMagnitude(),
		centered.ProjectOnto(GetForward(transform)).GetMagnitude());
}
Vector TransformUtils::WorldToLocal_Direction(Transform& transform, const Vector& vector)
{
	XMFLOAT3 worldDirection(vector.x, vector.y, vector.z);
	XMVECTOR worldDir = XMLoadFloat3(&worldDirection);

	//XMVECTOR localDirectionVector = XMVector3Rotate(worldDir, XMQuaternionInverse(XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
	XMVECTOR localDirectionVector = XMVector3Rotate(worldDir, XMQuaternionRotationRollPitchYaw(-transform.rotation.x * DEG2RAD, -transform.rotation.y * DEG2RAD, -transform.rotation.z * DEG2RAD));
	
	XMFLOAT3 localDirection;
	XMStoreFloat3(&localDirection, localDirectionVector);

	/*XMFLOAT4X4 worldMatrix = GetWorldMatrix();
	worldMatrix.
	worldMatrix.r[3] = XMVectorSet(0, 0, 0, 1);*/

	return Vector(localDirection);
}
#pragma endregion

#pragma region ColliderUtils
Matrix4x4 ColliderUtils::GetInertiaTensor(const Collider& collider, float mass)
{
	switch(collider.colliderType)
	{
		case EColliderType::None: break;
		case EColliderType::Box:
		{
			const BoxCollider& box = (const BoxCollider&) collider;
			return Matrix4x4::Scale(Vector(
				(1.0f / 12) * mass * (pow(2 * box.halfSize.y, 2) + pow(2 * box.halfSize.z, 2)),
				(1.0f / 12) * mass * (pow(2 * box.halfSize.x, 2) + pow(2 * box.halfSize.y, 2)),
				(1.0f / 12) * mass * (pow(2 * box.halfSize.x, 2) + pow(2 * box.halfSize.z, 2)),
				1));
		}
		break;
		case EColliderType::Sphere:
		{
			const SphereCollider& sphere = (const SphereCollider&) collider;
			return Matrix4x4::Identity() * ((2.0f / 3) * mass * sphere.radius * sphere.radius);
		}
		break;
		default: break;
	}
}
#pragma endregion

#pragma region RigidbodyUtils
namespace RigidbodyUtils
{
	void RigidbodyUtils::UpdatePhysics(Transform& transform, Rigidbody& rigidbody, float deltaTime)
	{
		// Minimum velocity allowed for a body's velocity to be considered non-zero. Basically acts as a global static coefficient of friction.
		static const float VELOCITY_THRESHOLD = 0.02f;

		// Velocity threshold to prevent small movements from accumulating
		if(rigidbody.velocity.GetMagnitude() < VELOCITY_THRESHOLD)
			rigidbody.velocity = Vector::Zero();
		if(rigidbody.angularVelocity.GetMagnitude() < VELOCITY_THRESHOLD)
			rigidbody.angularVelocity = Vector::Zero();

		TransformUtils::MoveAbsolute(transform, rigidbody.velocity * deltaTime);
		TransformUtils::RotateAxisAngle(transform, rigidbody.angularVelocity * deltaTime);

		rigidbody.velocity += rigidbody.netForce * deltaTime;
		rigidbody.angularVelocity += rigidbody.netTorque * deltaTime;

		rigidbody.netForce = Vector();
		rigidbody.netTorque = Vector();
	}

	void RigidbodyUtils::AddForce(Rigidbody& rigidbody, Vector force)
	{
		if(rigidbody.isStatic)
			return;

		rigidbody.netForce += force;
	}
	void RigidbodyUtils::AddTorque(Rigidbody& rigidbody, Vector axisAngle)
	{
		if(rigidbody.isStatic)
			return;

		rigidbody.netTorque += axisAngle;
	}
	void RigidbodyUtils::ApplyGravity(Rigidbody& rigidbody)
	{
		AddForce(rigidbody, rigidbody.gravity);
	}

	void RigidbodyUtils::ApplyImpulse(const Transform& transform, const Collider& collider, Rigidbody& rigidbody, const Vector& impulse, const Vector& location)
	{
		if(rigidbody.isStatic)
			return;

		// Apply linear impulse
		rigidbody.velocity += impulse / rigidbody.mass;
	
		Vector r = location - collider.center;

		// Apply angular impulse
		rigidbody.angularVelocity += GetInertiaTensor(transform, collider, rigidbody).Inverse() * r.Cross(impulse);
	}

	float RigidbodyUtils::GetMass(const Rigidbody& rigidbody)
	{
		// Static bodies should be treated as having infinite mass
		return rigidbody.isStatic ? FLT_MAX : rigidbody.mass;
	}
	float RigidbodyUtils::GetInverseMass(const Rigidbody& rigidbody)
	{
		return rigidbody.isStatic ? 0 : 1.0f / rigidbody.mass;
	}

	Matrix4x4 RigidbodyUtils::GetInertiaTensor(const Transform& transform, const Collider& collider, const Rigidbody& rigidbody)
	{
		Matrix4x4 inertiaTensor = ColliderUtils::GetInertiaTensor(collider, rigidbody.mass);

		return TransformUtils::GetRotationMatrix(transform) * (inertiaTensor * TransformUtils::GetRotationMatrix(transform).Transpose());
	}
}
#pragma endregion