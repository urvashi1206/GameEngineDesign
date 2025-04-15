#include "PhysicsUtils.hpp"

#include "Constants.h"

namespace Minimal
{
	#pragma region TransformUtils
	void TransformUtils::MoveAbsolute(TransformComponent& transform, glm::vec3 vector)
	{
		SetLocation(transform, transform.position + vector);
	}
	void TransformUtils::MoveRelative(TransformComponent& transform, glm::vec3 vector)
	{
		XMFLOAT3 currentLocation(transform.position.x, transform.position.y, transform.position.z);
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
	void TransformUtils::Rotate(TransformComponent& transform, glm::quat quaternion)
	{
		XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
		XMFLOAT4 f4_delta(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

		XMVECTOR xmv_result = XMQuaternionMultiply(XMLoadFloat4(&f4_rotation), XMLoadFloat4(&f4_delta));

		XMFLOAT4 f4_result;
		XMStoreFloat4(&f4_result, xmv_result);

		SetRotation(transform, f4_result);
	}
	void TransformUtils::RotateAxisAngle(TransformComponent& transform, glm::vec3 axisAngle)
	{
		if (axisAngle == 0)
			return;

		RotateAxisAngle(transform, axisAngle.Normalized(), axisAngle.GetMagnitude());
	}
	void TransformUtils::RotateAxisAngle(TransformComponent& transform, glm::vec3 axis, float angle)
	{
		if (axis == 0)
			return;

		DirectX::XMFLOAT3 f3_rotationAxis(axis.x, axis.y, axis.z);
		DirectX::XMVECTOR xmv_rot = DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&f3_rotationAxis), angle);

		XMFLOAT4 f4_rotation(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
		XMVECTOR xmv_rotation = DirectX::XMQuaternionMultiply(XMLoadFloat4(&f4_rotation), xmv_rot);

		XMStoreFloat4(&f4_rotation, xmv_rotation);

		SetRotation(transform, f4_rotation);
	}
	void TransformUtils::RotatePitchYawRoll(TransformComponent& transform, glm::vec3 pitchYawRoll)
	{
		SetRotationPitchYawRoll(transform, GetPitchYawRoll(transform) + pitchYawRoll);

		/*XMFLOAT4 f4_rot;
		XMStoreFloat4(&f4_rot, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));

		Rotate(f4_rot);*/
	}
	void TransformUtils::Scale(TransformComponent& transform, glm::vec3 vector)
	{
		SetScale(transform, transform.scale * vector);
	}

	#pragma region Getters
	glm::vec3 TransformUtils::GetPitchYawRoll(const TransformComponent& transform)
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

		return glm::vec3(pitch, yaw, roll); // Returned in radians
	}

	glm::mat4 TransformUtils::GetRotationMatrix(const TransformComponent& transform)
	{
		return Matrix4x4(GetRight(transform), GetUp(transform), GetForward(transform));
	}
	#pragma endregion

	#pragma region Setters
	void TransformUtils::SetLocation(TransformComponent& transform, glm::vec3 location)
	{
		transform.position = location;

		transform.isWorldMatrixDirty = true;
	}
	void TransformUtils::SetRotation(TransformComponent& transform, glm::quat quaternion)
	{
		transform.rotation = quaternion;

		transform.isWorldMatrixDirty = true;
	}
	void TransformUtils::SetRotationPitchYawRoll(TransformComponent& transform, glm::vec3 pitchYawRoll)
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
	#pragma endregion

	glm::vec3 TransformUtils::LocalToWorld_Point(const TransformComponent& transform, const glm::vec3& vector, bool includeScale)
	{
		return transform.position + transform.right() * vector.x + transform.up() * vector.y + transform.forward() * vector.z;
	}
	glm::vec3 TransformUtils::LocalToWorld_Direction(const TransformComponent& transform, const glm::vec3& vector, bool includeScale)
	{
		return transform.right() * vector.x + transform.up() * vector.y + transform.forward() * vector.z;
	}
	#pragma endregion

	#pragma region ColliderUtils
	glm::mat4 ColliderUtils::GetInertiaTensor(const ColliderComponent& collider, float mass)
	{
		switch (collider.colliderType)
		{
		case EColliderType::None: break;
		case EColliderType::Box:
		{
			const BoxCollider& box = (const BoxCollider&)collider;
			return Matrix4x4::Scale(Vector(
				(1.0f / 12) * mass * (pow(2 * box.halfSize.y, 2) + pow(2 * box.halfSize.z, 2)),
				(1.0f / 12) * mass * (pow(2 * box.halfSize.x, 2) + pow(2 * box.halfSize.y, 2)),
				(1.0f / 12) * mass * (pow(2 * box.halfSize.x, 2) + pow(2 * box.halfSize.z, 2)),
				1));
		}
		break;
		case EColliderType::Sphere:
		{
			const SphereCollider& sphere = (const SphereCollider&)collider;
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
		void RigidbodyUtils::UpdatePhysics(TransformComponent& transform, RigidbodyComponent& rigidbody, float deltaTime)
		{
			// Minimum velocity allowed for a body's velocity to be considered non-zero. Basically acts as a global static coefficient of friction.
			static const float VELOCITY_THRESHOLD = 0.02f;

			// Velocity threshold to prevent small movements from accumulating
			if (rigidbody.velocity.GetMagnitude() < VELOCITY_THRESHOLD)
				rigidbody.velocity = Vector::Zero();
			if (rigidbody.angularVelocity.GetMagnitude() < VELOCITY_THRESHOLD)
				rigidbody.angularVelocity = Vector::Zero();

			TransformUtils::MoveAbsolute(transform, rigidbody.velocity * deltaTime);
			TransformUtils::RotateAxisAngle(transform, rigidbody.angularVelocity * deltaTime);

			rigidbody.velocity += rigidbody.netForce * deltaTime;
			rigidbody.angularVelocity += rigidbody.netTorque * deltaTime;

			rigidbody.netForce = glm::vec3();
			rigidbody.netTorque = glm::vec3();
		}

		void RigidbodyUtils::AddForce(RigidbodyComponent& rigidbody, glm::vec3 force)
		{
			if (rigidbody.isStatic)
				return;

			rigidbody.netForce += force;
		}
		void RigidbodyUtils::AddTorque(RigidbodyComponent& rigidbody, glm::vec3 axisAngle)
		{
			if (rigidbody.isStatic)
				return;

			rigidbody.netTorque += axisAngle;
		}
		void RigidbodyUtils::ApplyGravity(RigidbodyComponent& rigidbody)
		{
			AddForce(rigidbody, rigidbody.gravity);
		}

		void RigidbodyUtils::ApplyImpulse(const TransformComponent& transform, const ColliderComponent& collider, RigidbodyComponent& rigidbody, const Vector& impulse, const Vector& location)
		{
			if (rigidbody.isStatic)
				return;

			// Apply linear impulse
			rigidbody.velocity += impulse / rigidbody.mass;

			glm::vec3 r = location - collider.center;

			// Apply angular impulse
			rigidbody.angularVelocity += GetInertiaTensor(transform, collider, rigidbody).Inverse() * r.Cross(impulse);
		}

		float RigidbodyUtils::GetMass(const RigidbodyComponent& rigidbody)
		{
			// Static bodies should be treated as having infinite mass
			return rigidbody.isStatic ? FLT_MAX : rigidbody.mass;
		}
		float RigidbodyUtils::GetInverseMass(const RigidbodyComponent& rigidbody)
		{
			return rigidbody.isStatic ? 0 : 1.0f / rigidbody.mass;
		}

		glm::mat4 RigidbodyUtils::GetInertiaTensor(const TransformComponent& transform, const ColliderComponent& collider, const RigidbodyComponent& rigidbody)
		{
			glm::mat4 inertiaTensor = ColliderUtils::GetInertiaTensor(collider, rigidbody.mass);

			return TransformUtils::GetRotationMatrix(transform) * (inertiaTensor * TransformUtils::GetRotationMatrix(transform).Transpose());
		}
	}
	#pragma endregion
}