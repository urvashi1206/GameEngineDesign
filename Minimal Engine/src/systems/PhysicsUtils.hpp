#pragma once

#include "ecs/Components.hpp"

namespace Minimal
{
	namespace TransformUtils
	{
		static void MoveRelative(TransformComponent& transform, glm::vec3 vector);

		static glm::mat4 GetRotationMatrix(const TransformComponent& transform);

		static glm::vec3 LocalToWorld_Point(const TransformComponent& transform, const glm::vec3& vector, bool includeScale = false) {
			return transform.position + transform.right() * vector.x + transform.up() * vector.y + transform.forward() * vector.z;
		};
		static glm::vec3 LocalToWorld_Direction(const TransformComponent& transform, const glm::vec3& vector, bool includeScale = false);
	}

	namespace ColliderUtils
	{
		glm::vec3 GetCenter(const TransformComponent& transform, const ColliderComponent& collider);
		glm::mat4 GetInertiaTensor(const ColliderComponent& collider, float mass);
	}

	namespace RigidbodyUtils
	{
		void UpdatePhysics(TransformComponent& transform, RigidbodyComponent& rigidbody, float deltaTime);

		void AddForce(RigidbodyComponent& rigidbody, glm::vec3 force);
		void AddTorque(RigidbodyComponent& rigidbody, glm::vec3 axisAngle);
		void ApplyGravity(RigidbodyComponent& rigidbody);

		void ApplyImpulse(const TransformComponent& transform, const ColliderComponent& collider, RigidbodyComponent& rigidbody, const glm::vec3& impulse, const glm::vec3& location);

		float GetMass(const RigidbodyComponent& rigidbody);
		float GetInverseMass(const RigidbodyComponent& rigidbody);

		glm::mat4 GetInertiaTensor(const TransformComponent& transform, const ColliderComponent& collider, const RigidbodyComponent& rigidbody);
	}
}