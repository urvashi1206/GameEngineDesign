#include "PhysicsUtils.hpp"

namespace Minimal
{
	#pragma region TransformUtils
	void TransformUtils::MoveRelative(TransformComponent& transform, glm::vec3 vector)
	{
		transform.position += transform.rotation * vector;
	}

	#pragma region Getters
	glm::mat4 TransformUtils::GetRotationMatrix(const TransformComponent& transform)
	{
		return glm::transpose(glm::mat4(
			glm::vec4(transform.right(), 1), 
			glm::vec4(transform.up(), 1), 
			glm::vec4(transform.forward(), 1), 
			glm::vec4(0, 0, 0, 1)));
	}
	#pragma endregion

	/*glm::vec3 TransformUtils::LocalToWorld_Point(const TransformComponent& transform, const glm::vec3& vector, bool includeScale)
	{
		return transform.position + transform.right() * vector.x + transform.up() * vector.y + transform.forward() * vector.z;
	}*/
	glm::vec3 TransformUtils::LocalToWorld_Direction(const TransformComponent& transform, const glm::vec3& vector, bool includeScale)
	{
		return transform.right() * vector.x + transform.up() * vector.y + transform.forward() * vector.z;
	}
	#pragma endregion

	#pragma region ColliderUtils
	glm::vec3 ColliderUtils::GetCenter(const TransformComponent& transform, const ColliderComponent& collider)
	{
		return TransformUtils::LocalToWorld_Point(transform, collider.center);
	}
	glm::mat4 ColliderUtils::GetInertiaTensor(const ColliderComponent& collider, float mass)
	{
		switch (collider.colliderType)
		{
		case EColliderType::None: break;
		case EColliderType::Box:
		{
			return glm::scale(glm::mat4(1.0f), glm::vec3(
				(1.0f / 12) * mass * (pow(2 * collider.halfSize.x, 2) + pow(2 * collider.halfSize.y, 2)),
				(1.0f / 12) * mass * (pow(2 * collider.halfSize.x, 2) + pow(2 * collider.halfSize.z, 2)),
				(1.0f / 12) * mass * (pow(2 * collider.halfSize.y, 2) + pow(2 * collider.halfSize.z, 2))));
		}
		break;
		case EColliderType::Sphere:
		{
			return glm::mat4(1.0f) * ((2.0f / 3) * mass * collider.radius * collider.radius);
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
			static const float VELOCITY_THRESHOLD = 0.01f;

			// Velocity threshold to prevent small movements from accumulating
			if (glm::length(rigidbody.velocity) < VELOCITY_THRESHOLD)
				rigidbody.velocity = glm::vec3(0, 0, 0);
			if (glm::length(rigidbody.angularVelocity) < VELOCITY_THRESHOLD)
				rigidbody.angularVelocity = glm::vec3(0, 0, 0);

			transform.position += rigidbody.velocity * deltaTime;
			if(glm::length(rigidbody.angularVelocity) > 0)
				transform.rotate(glm::angleAxis(glm::length(rigidbody.angularVelocity) * deltaTime, glm::normalize(rigidbody.angularVelocity)));

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

		void RigidbodyUtils::ApplyImpulse(const TransformComponent& transform, const ColliderComponent& collider, RigidbodyComponent& rigidbody, const glm::vec3& impulse, const glm::vec3& location)
		{
			if (rigidbody.isStatic)
				return;

			// Apply linear impulse
			rigidbody.velocity += impulse / rigidbody.mass;

			glm::vec3 r = location - ColliderUtils::GetCenter(transform, collider);

			// Apply angular impulse
			glm::vec4 cross = glm::vec4(glm::cross(r, impulse), 0);
			glm::mat4 inv = glm::inverse(GetInertiaTensor(transform, collider, rigidbody));
			glm::vec3 delta = (glm::vec3)(inv * cross);
			rigidbody.angularVelocity += delta;
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

			return TransformUtils::GetRotationMatrix(transform) * (inertiaTensor * glm::transpose(TransformUtils::GetRotationMatrix(transform)));
		}
	}
	#pragma endregion
}