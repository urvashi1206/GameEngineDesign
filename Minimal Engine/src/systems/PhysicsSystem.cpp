#include "PhysicsSystem.hpp"
#include "PhysicsUtils.hpp"

#include <algorithm>
#include <iostream>

namespace Minimal
{
	PhysicsSystem::PhysicsSystem(ECSCoordinator& ecs) : System(ecs) {}
	
	void PhysicsSystem::initialize() {
		
	}

	void PhysicsSystem::update(FrameInfo& frameInfo) {
		static const float PHYSICS_TICK = 0.01f;

		static float simulationTimeLeft = 0;

		simulationTimeLeft += frameInfo.frameTime;

		while (simulationTimeLeft >= PHYSICS_TICK)
		{
			std::vector<CollisionData> collisions;

			/* Physics Update */

			for (Entity e = 0; e < 10; e++)
				if (m_ecs.hasComponent<RigidbodyComponent>(e) && m_ecs.hasComponent<ColliderComponent>(e))
				{
					TransformComponent& transform = m_ecs.getComponent<TransformComponent>(e);
					RigidbodyComponent& rb = m_ecs.getComponent<RigidbodyComponent>(e);

					RigidbodyUtils::ApplyGravity(rb);

					RigidbodyUtils::UpdatePhysics(transform, rb, tickPhysics ? PHYSICS_TICK : 0);

					std::cout << transform.position.x << " " << transform.position.y << " " << transform.position.z << std::endl;
				}

			/* Collision Detection and Contact Generation */

			for (Entity e1 = 0; e1 < 10; e1++)
			{
				if (!m_ecs.hasComponent<RigidbodyComponent>(e1) || !m_ecs.hasComponent<ColliderComponent>(e1))
					continue;
				
				for (Entity e2 = 0; e2 < 10; e2++)
				{
					if (!m_ecs.hasComponent<RigidbodyComponent>(e2) || !m_ecs.hasComponent<ColliderComponent>(e2))
						continue;

					if (e1 == e2)
						continue;

					TransformComponent& transformA = m_ecs.getComponent<TransformComponent>(e1);
					TransformComponent& transformB = m_ecs.getComponent<TransformComponent>(e2);
					ColliderComponent& colliderA = m_ecs.getComponent<ColliderComponent>(e1);
					ColliderComponent& colliderB = m_ecs.getComponent<ColliderComponent>(e2);

					std::vector<ContactPoint> contactPoints;
					if (GJK(transformA, colliderA, transformB, colliderB, contactPoints))
					{
						CollisionData collisionData{};
						collisionData.entityA = e1;
						collisionData.entityB = e2;
						collisionData.colliderPair = CollisionPair(&colliderA, &colliderB);
						collisionData.contacts = contactPoints;

						collisions.push_back(collisionData);
					}
				}
			}

			/* Collision Resolution */

			for (CollisionData& collisionData : collisions)
			{
				Entity entityA = collisionData.entityA;
				Entity entityB = collisionData.entityB;

				RigidbodyComponent& bodyA = m_ecs.getComponent<RigidbodyComponent>(entityA);
				RigidbodyComponent& bodyB = m_ecs.getComponent<RigidbodyComponent>(entityB);
				const ColliderComponent& colliderA = *collisionData.colliderPair.GetFirst();
				const ColliderComponent& colliderB = *collisionData.colliderPair.GetSecond();
				TransformComponent& transformA = m_ecs.getComponent<TransformComponent>(entityA);
				TransformComponent& transformB = m_ecs.getComponent<TransformComponent>(entityB);

				glm::vec3 initialLocationA = ColliderUtils::GetCenter(transformA, colliderA);
				glm::vec3 initialLocationB = ColliderUtils::GetCenter(transformB, colliderB);
				glm::vec3 initialVelocityA = bodyA.velocity;
				glm::vec3 initialVelocityB = bodyB.velocity;
				glm::vec3 initialAngularVelocityA = bodyA.angularVelocity;
				glm::vec3 initialAngularVelocityB = bodyB.angularVelocity;

				float inverseMassA = RigidbodyUtils::GetInverseMass(bodyA);
				float inverseMassB = RigidbodyUtils::GetInverseMass(bodyB);
				glm::mat4 inertiaTensorA = RigidbodyUtils::GetInertiaTensor(transformA, colliderA, bodyA);
				glm::mat4 inertiaTensorB = RigidbodyUtils::GetInertiaTensor(transformB, colliderB, bodyB);

				for (const ContactPoint& contact : collisionData.contacts)
				{
					//Debug::CreateWireframe_Temp(contact.location, Vector(), Vector(0.1f, 0.1f, 0.1f));

					// Location and velocity relative to center of mass
					glm::vec3 rA = contact.location - ColliderUtils::GetCenter(transformA, colliderA);
					glm::vec3 rB = contact.location - ColliderUtils::GetCenter(transformB, colliderB);
					glm::vec3 relativeVelocity = bodyB.velocity + glm::cross(bodyB.angularVelocity, rB) - bodyA.velocity - glm::cross(bodyA.angularVelocity, rA);

					// Calculate relative velocity along the normal
					float velocityAlongNormal = glm::dot(relativeVelocity, contact.normal);

					// If the velocities are separating, no need to resolve
					if (velocityAlongNormal > 0)
						continue;

					// Calculate restitution (bounciness)
					float e = std::min(bodyA.bounciness, bodyB.bounciness);

					// Calculate impulse scalar
					float j = -(1 + e) * velocityAlongNormal;
					j /= inverseMassA + inverseMassB +
						glm::dot(contact.normal, glm::cross((glm::vec3) (glm::inverse(inertiaTensorA) * glm::vec4(glm::cross(rA, contact.normal), 0)), rA)) +
						glm::dot(contact.normal, glm::cross((glm::vec3) (glm::inverse(inertiaTensorB) * glm::vec4(glm::cross(rB, contact.normal), 0)), rB));

					// Apply impulse
					glm::vec3 impulse = contact.normal * j;
					RigidbodyUtils::ApplyImpulse(transformA, colliderA, bodyA, -impulse, contact.location);
					RigidbodyUtils::ApplyImpulse(transformB, colliderB, bodyB, impulse, contact.location);

					/* Friction */

					// Use original location and velocity values for friction
					rA = contact.location - initialLocationA;
					rB = contact.location - initialLocationB;
					relativeVelocity = initialVelocityB + glm::cross(initialAngularVelocityB, rB) - initialVelocityA - glm::cross(initialAngularVelocityA, rA);

					// Determine tangential velocity
					glm::vec3 tangentialVelocity = relativeVelocity - contact.normal * velocityAlongNormal;
					if (glm::dot(tangentialVelocity, tangentialVelocity) > 1e-6f)
						tangentialVelocity = glm::normalize(tangentialVelocity);
					else
						tangentialVelocity = glm::vec3(0, 0, 0);

					// Determine whether to use static or dynamic friction and scale max impulse
					float coefficient = tangentialVelocity == glm::vec3(0, 0, 0) ?
						sqrt(pow(bodyA.staticFriction, 2) + pow(bodyB.staticFriction, 2))
						: sqrt(pow(bodyA.dynamicFriction, 2) + pow(bodyB.dynamicFriction, 2));
					float maxFrictionImpulse = coefficient * j;

					glm::vec3 rA_cross_t = glm::cross(rA, tangentialVelocity);
					glm::vec3 rB_cross_t = glm::cross(rB, tangentialVelocity);
					float angularEffectFrictionA = glm::dot(rA_cross_t, (glm::vec3) (glm::inverse(inertiaTensorA) * glm::vec4(rA_cross_t, 0)));
					float angularEffectFrictionB = glm::dot(rB_cross_t, (glm::vec3) (glm::inverse(inertiaTensorB) * glm::vec4(rB_cross_t, 0)));

					float frictionInertia = inverseMassA + inverseMassB + angularEffectFrictionA + angularEffectFrictionB;
					float frictionImpulseMagnitude = glm::dot(-relativeVelocity, tangentialVelocity) / frictionInertia;
					frictionImpulseMagnitude = std::clamp(frictionImpulseMagnitude, -abs(maxFrictionImpulse), abs(maxFrictionImpulse));

					glm::vec3 frictionImpulse = tangentialVelocity * frictionImpulseMagnitude;

					// Apply friction impulse
					RigidbodyUtils::ApplyImpulse(transformA, colliderA, bodyA, -frictionImpulse, contact.location);
					RigidbodyUtils::ApplyImpulse(transformB, colliderB, bodyB, frictionImpulse, contact.location);

					// Positional correction to prevent sinking
					const float percent = 0.2f; // Usually 20% to 80%
					const float slop = 0.01f; // Usually 0.01 to 0.1
					glm::vec3 correction = contact.normal * std::max(contact.penetrationDepth - slop, 0.0f) / (inverseMassA + inverseMassB) * percent;
					transformA.position -= correction * inverseMassA;
					transformB.position += correction * inverseMassB;
				}

				//Pause();

				//break;
			}

			simulationTimeLeft -= PHYSICS_TICK;
		}
	}

	bool PhysicsSystem::GJK(const TransformComponent& transformA, const ColliderComponent& a, const TransformComponent& transformB, const ColliderComponent& b, std::vector<ContactPoint>& out_contactPoints)
	{
		// Arbitrary direction as a starting point
		glm::vec3 direction(1, 0, 0);
		//assert((a->GJK_Support(direction) - b->GJK_Support(-direction)) == -(b->GJK_Support(direction) - a->GJK_Support(-direction)));
		//assert();
		// Get the Minkowski Difference on the hull in this direction
		glm::vec3 difference = GJK_Support(transformA, a, direction) - GJK_Support(transformB, b, -direction);
		// Initialize the simplex
		std::deque<glm::vec3> simplex{ difference };
		std::unordered_map<glm::vec3, std::pair<glm::vec3, glm::vec3>> supportPoints;

		// Get the new direction towards the origin
		direction = -simplex.back();

		// Add up to three more points to the simplex to try to contain the origin
		while (true)
		{
			// Get the next Minkowski Difference on the hull
			glm::vec3 support = GJK_Support(transformA, a, direction) - GJK_Support(transformB, b, -direction);
			// If we didn't pass the origin, return false
			if (glm::dot(support, direction) <= 0)
				return false;

			// Add new point to simplex
			simplex.push_front(support);
			supportPoints.emplace(support, std::make_pair<glm::vec3, glm::vec3>(GJK_Support(transformA, a, direction), GJK_Support(transformB, b, -direction)));

			// Test for the simplex intersecting the origin
			if (UpdateSimplex(simplex, direction))
			{
				// Perform EPA to determine collision normal
				out_contactPoints = EPA(simplex, supportPoints, transformA, a, transformB, b);

				return true;
			}
		}
	}
	bool PhysicsSystem::UpdateSimplex(std::deque<glm::vec3>& simplex, glm::vec3& direction)
	{
		switch (simplex.size())
		{
		case 2: // Line test
			return UpdateSimplex_LineCase(simplex, direction);
		case 3: // Triangle test
			return UpdateSimplex_TriangleCase(simplex, direction);
		case 4: // Tetrahedron test
			return UpdateSimplex_TetrahedronCase(simplex, direction);
		default: return false;
		}
	}

	bool PhysicsSystem::UpdateSimplex_LineCase(std::deque<glm::vec3>& simplex, glm::vec3& direction)
	{
		glm::vec3 a = simplex[0];
		glm::vec3 b = simplex[1];

		glm::vec3 ao = -a;
		glm::vec3 ab = b - a; // The only edge

		if (glm::dot(ab, ao) > 0)
			direction = glm::cross(glm::cross(ab, ao), ab);
		else
		{
			simplex = { a };
			direction = ao;
		}

		return false;
	}
	bool PhysicsSystem::UpdateSimplex_TriangleCase(std::deque<glm::vec3>& simplex, glm::vec3& direction)
	{
		glm::vec3 a = simplex[0];
		glm::vec3 b = simplex[1];
		glm::vec3 c = simplex[2];

		glm::vec3 ao = -a;
		glm::vec3 ab = b - a; // Edge 1
		glm::vec3 ac = c - a; // Edge 2

		glm::vec3 abc = glm::cross(ab, ac); // Normal of the triangle

		if (glm::dot(glm::cross(abc, ac), ao) > 0)
		{
			if (glm::dot(ac, ao) > 0)
			{
				simplex = { a, c };
				direction = glm::cross(glm::cross(ac, ao), ac);
			}
			else
				UpdateSimplex_LineCase(simplex = { a, b }, direction);
		}
		else
		{
			if (glm::dot(glm::cross(ab, abc), ao) > 0)
				return UpdateSimplex_LineCase(simplex = { a, b }, direction);
			else
			{
				if (glm::dot(abc, ao) > 0)
				{
					direction = abc;
				}
				else
				{
					simplex = { a, c, b };
					direction = -abc;
				}
			}
		}

		return false;
	}
	bool PhysicsSystem::UpdateSimplex_TetrahedronCase(std::deque<glm::vec3>& simplex, glm::vec3& direction)
	{
		glm::vec3 a = simplex[0];
		glm::vec3 b = simplex[1];
		glm::vec3 c = simplex[2];
		glm::vec3 d = simplex[3];

		glm::vec3 ao = -a;
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ad = d - a;

		glm::vec3 abc = glm::cross(ab, ac);
		glm::vec3 acd = glm::cross(ac, ad);
		glm::vec3 adb = glm::cross(ad, ab);

		if (glm::dot(abc, ao) > 0)
			return UpdateSimplex_TriangleCase(simplex = { a, b, c }, direction);
		if (glm::dot(acd, ao) > 0)
			return UpdateSimplex_TriangleCase(simplex = { a, c, d }, direction);
		if (glm::dot(adb, ao) > 0)
			return UpdateSimplex_TriangleCase(simplex = { a, d, b }, direction);

		return true;
	}

	std::vector<ContactPoint> PhysicsSystem::EPA(std::deque<glm::vec3> simplex, std::unordered_map<glm::vec3, std::pair<glm::vec3, glm::vec3>> supportPoints, const TransformComponent& transformA, const ColliderComponent& a, const TransformComponent& transformB, const ColliderComponent& b)
	{
		// The acceptable range for a point to be considered on the boundary of the Minkowski Difference
		static const float EDGE_TOLERANCE = 0.01f;

		// The polytope, represented by a series of vertices.
		std::vector<glm::vec3> polytope(simplex.begin(), simplex.end());
		// The faces of the polytope, represented by index triplets indicating the vertices in each face.
		std::vector<unsigned int> faces =
		{
			0, 1, 2,
			0, 3, 1,
			0, 2, 3,
			1, 3, 2
		};

		// Normals and distances
		std::vector<std::pair<glm::vec3, float>> normals;
		unsigned int minNormalIndex;
		EPA_GetPolytopeNormals(polytope, faces, normals, minNormalIndex);

		glm::vec3 minNormal;
		float minDistance = FLT_MAX;
		while (minDistance == FLT_MAX)
		{
			minNormal = normals[minNormalIndex].first;
			minDistance = normals[minNormalIndex].second;

			glm::vec3 supportPoint = GJK_Support(transformA, a, minNormal) - GJK_Support(transformB, b, -minNormal);
			float sDistance = glm::dot(minNormal, supportPoint);

			// If the polytope already contains the support point, terminate - we have found the closest face
			if (std::find(polytope.begin(), polytope.end(), supportPoint) != polytope.end())
				break;

			if (abs(sDistance - minDistance) > EDGE_TOLERANCE)
			{
				minDistance = FLT_MAX;

				std::vector<std::pair<unsigned int, unsigned int>> uniqueEdges;
				for (unsigned int i = 0; i < normals.size(); i++)
				{
					if (glm::dot(normals[i].first, supportPoint) > 0)
					{
						unsigned int f = i * 3;

						EPA_AddIfUniqueEdge(uniqueEdges, faces, f, f + 1);
						EPA_AddIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
						EPA_AddIfUniqueEdge(uniqueEdges, faces, f + 2, f);

						faces[f + 2] = faces.back();
						faces.pop_back();
						faces[f + 1] = faces.back();
						faces.pop_back();
						faces[f] = faces.back();
						faces.pop_back();

						normals[i] = normals.back();
						normals.pop_back();

						i--;
					}
				}

				std::vector<unsigned int> newFaces;
				for (auto [edgeIndex1, edgeIndex2] : uniqueEdges)
				{
					newFaces.push_back(edgeIndex1);
					newFaces.push_back(edgeIndex2);
					newFaces.push_back(polytope.size());
				}

				polytope.push_back(supportPoint);
				supportPoints.emplace(supportPoint, std::make_pair<glm::vec3, glm::vec3>(GJK_Support(transformA, a, minNormal), GJK_Support(transformB, b, -minNormal)));

				std::vector<std::pair<glm::vec3, float>> newNormals;
				unsigned int newMinFace;
				EPA_GetPolytopeNormals(polytope, newFaces, newNormals, newMinFace);

				float oldMinDistance = FLT_MAX;
				for (size_t i = 0; i < normals.size(); i++)
				{
					if (normals[i].second < oldMinDistance)
					{
						oldMinDistance = normals[i].second;
						minNormalIndex = i;
					}
				}

				if (newNormals[newMinFace].second < oldMinDistance)
					minNormalIndex = newMinFace + normals.size();

				faces.insert(faces.end(), newFaces.begin(), newFaces.end());
				normals.insert(normals.end(), newNormals.begin(), newNormals.end());
			}
		}

		std::vector<ContactPoint> contactPoints;

		/* Generate contact points */

		// Points on the closest face of the polytope
		glm::vec3 A = polytope[faces[minNormalIndex * 3]];
		glm::vec3 B = polytope[faces[minNormalIndex * 3 + 1]];
		glm::vec3 C = polytope[faces[minNormalIndex * 3 + 2]];

		// Project origin onto contact plane
		//glm::vec3 P = Vector().ProjectOntoPlane(A, minNormal);

		//// The corresponding support points on the original colliders
		//glm::vec3 Aa = supportPoints[A].first;
		//glm::vec3 Ab = supportPoints[B].first;
		//glm::vec3 Ac = supportPoints[C].first;

		//glm::vec3 Ba = supportPoints[A].second;
		//glm::vec3 Bb = supportPoints[B].second;
		//glm::vec3 Bc = supportPoints[C].second;

		//// Areas of the triangles in the face with the projected origin (for calculating barycentrics)
		//float areaPBC = glm::dot(glm::cross(B - P, C - P), glm::normalize(glm::cross(B - A, C - A)));
		//float areaPCA = glm::dot(glm::cross(C - P, A - P), glm::normalize(glm::cross(B - A, C - A)));
		//float areaABC = glm::dot(glm::cross(B - A, C - A), glm::normalize(glm::cross(B - A, C - A)));

		//// Barycentric coordinates
		//float alpha = areaPBC / areaABC;
		//float beta = areaPCA / areaABC;
		//float gamma = 1 - alpha - beta;

		//glm::vec3 l = A * alpha + B * beta + C * gamma;
		////assert((l - P).GetMagnitude() < 0.1f);

		//// Contact point on collider A
		//glm::vec3 Ap = Aa * alpha + Ab * beta + Ac * gamma;
		//// Contact point on collider B
		//glm::vec3 Bp = Ba * alpha + Bb * beta + Bc * gamma;

		////Debug::CreateWireframe_Temp(Ap, Vector(), Vector(0.25f, 0.25f, 0.25f));
		////Debug::CreateWireframe_Temp(Bp, Vector(), Vector(0.25f, 0.25f, 0.25f));
		////Debug::CreateWireframe_Temp((Ap + Bp) / 2, Vector(), Vector(0.25f, 0.25f, 0.25f)); // The midpoint between the contact points on each collider is the final contact point

		////Debug::CreateWireframe_Temp(Aa, Vector(), Vector(0.125f, 0.125f, 0.125f));
		////Debug::CreateWireframe_Temp(Ab, Vector(), Vector(0.125f, 0.125f, 0.125f));
		////Debug::CreateWireframe_Temp(Ac, Vector(), Vector(0.125f, 0.125f, 0.125f));
		////Debug::CreateWireframe_Temp(Ap, Vector(), Vector(0.125f, 0.125f, 0.125f));
		////Debug::CreateWireframe_Temp(P, Vector(), Vector(0.125f, 0.125f, 0.125f));
		////Debug::CreateWireframe_Temp(l, Vector(), Vector(0.125f, 0.125f, 0.125f));

		//// For now, only return the contact point for collider A
		//ContactPoint contactPoint;
		//contactPoint.location = (Ap + Bp) / glm::vec3(2, 2, 2);
		//contactPoint.normal = minNormal;
		//contactPoint.penetrationDepth = minDistance + EDGE_TOLERANCE;

		//contactPoints.push_back(contactPoint);

		glm::vec3 normalA;
		std::vector<glm::vec3> faceA = EPA_GetAlignedFace(transformA, a, minNormal, normalA);
		glm::vec3 normalB;
		std::vector<glm::vec3> faceB = EPA_GetAlignedFace(transformB, b, -minNormal, normalB);

		std::vector<glm::vec3> clipAB = GetContactPoints(faceA, faceB, normalB);
		std::vector<glm::vec3> clipBA = GetContactPoints(faceB, faceA, normalA);

		std::vector<glm::vec3> contacts;
		for (glm::vec3& p : clipAB)
			contacts.push_back(p);
		for (glm::vec3& p : clipBA)
			contacts.push_back(p);

		for (glm::vec3 point : clipAB)
		{
			ContactPoint contact = {};
			contact.location = point;
			contact.normal = minNormal;
			contact.penetrationDepth = minDistance + EDGE_TOLERANCE;

			contactPoints.push_back(contact);

			//if(a->transform->GetScale().x <= 0.5f)
				//Debug::CreateWireframe_Temp(point, Vector(), Vector(0.125f, 0.125f, 0.125f));
		}

		//assert(contactPoints.size() > 0);
		//if(contactPoints.size() == 0)
			//PhysicsSubsystem::Pause();

		/*for(Vector point : { Aa, Ab, Ac })
		{
			ContactPoint contact = {};
			contact.location = point;
			contact.normal = minNormal;
			contact.penetrationDepth = minDistance + EDGE_TOLERANCE;

			contactPoints.push_back(contact);
		}*/

		// Check cached contacts for similar values; don't update if not necessary
		//CollisionPair colliderPair(&a, &b);
		//if (AreContactsValidInCache(colliderPair, contactPoints))
		//	return cachedContacts[colliderPair];

		//// Cache the contacts between these two colliders so they aren't regenerated if there is no significant difference in locations ("resting contact")
		//cachedContacts[colliderPair] = contactPoints;

		return contactPoints;
	}
	void PhysicsSystem::EPA_GetPolytopeNormals(std::vector<glm::vec3> polytope, std::vector<unsigned int> faces, std::vector<std::pair<glm::vec3, float>>& out_normals, unsigned int& out_minNormalIndex)
	{
		// Initial index of the minimum distance triangle
		unsigned int minTriangleIndex = 0;
		float minDistance = FLT_MAX;

		for (unsigned int i = 0; i < faces.size(); i += 3)
		{
			glm::vec3 a = polytope[faces[i]];
			glm::vec3 b = polytope[faces[i + 1]];
			glm::vec3 c = polytope[faces[i + 2]];

			glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
			float distance = glm::dot(normal, a);
			// Reverse normal if facing inward
			if (distance < 0)
			{
				normal = -normal;
				distance = -distance;
			}

			// Update out variables
			out_normals.emplace_back(normal, distance);

			if (distance < minDistance)
			{
				minTriangleIndex = i / 3;
				minDistance = distance;
			}
		}

		out_minNormalIndex = minTriangleIndex;
	}
	void PhysicsSystem::EPA_AddIfUniqueEdge(std::vector<std::pair<unsigned int, unsigned int>>& edges, const std::vector<unsigned int>& faces, unsigned int a, unsigned int b)
	{
		auto reverse = std::find(edges.begin(), edges.end(), std::make_pair(faces[b], faces[a]));

		if (reverse != edges.end())
			edges.erase(reverse);
		else
			edges.emplace_back(faces[a], faces[b]);
	}

	std::vector<glm::vec3> PhysicsSystem::GetContactPoints(std::vector<glm::vec3> incidentFace, std::vector<glm::vec3> referenceFace, glm::vec3 referenceNormal)
	{
		static const float THICKNESS = 0.001f;

		if (incidentFace.size() == 1)
			return incidentFace;
		if (referenceFace.size() == 1)
			return referenceFace;

		assert(incidentFace.size() > 2 && referenceFace.size() > 2);

		glm::vec3 incidentNormal = glm::normalize(glm::cross(incidentFace[1] - incidentFace[0], incidentFace[2] - incidentFace[0]));

		glm::vec3 referenceCenter = glm::vec3(0, 0, 0);
		for (int i = 0; i < referenceFace.size(); i++)
			referenceCenter += referenceFace[i];
		referenceCenter /= referenceFace.size();

		std::unordered_map<glm::vec3, float> edgePlanes;
		for (size_t i = 0; i < referenceFace.size(); i++)
		{
			glm::vec3 currentVertex = referenceFace[i];
			glm::vec3 nextVertex = referenceFace[(i + 1) % referenceFace.size()];

			glm::vec3 currentToNext = nextVertex - currentVertex;
			glm::vec3 cross = glm::cross(currentToNext, referenceNormal);

			glm::vec3 edgeNormal = glm::normalize(cross);
			float planeOffset = glm::dot(edgeNormal, currentVertex);
			glm::vec3 l = currentVertex - referenceCenter;
			float dot = glm::dot(edgeNormal, l);
			if (glm::dot(edgeNormal, currentVertex - referenceCenter) >= 0)
			{
				edgeNormal = -edgeNormal;
				planeOffset = -planeOffset;
			}
			if(edgePlanes.find(edgeNormal) != edgePlanes.end())
				int k = 0;
			edgePlanes.emplace(edgeNormal, planeOffset);
		}

		// All edge normals should be unique for convex shapes; if not, vertex order is incorrect
		assert(edgePlanes.size() == referenceFace.size());

		float referencePlaneOffset = glm::dot(referenceNormal, referenceFace[0]);
		float k = (referencePlaneOffset == 0 ? 0 : referencePlaneOffset / abs(referencePlaneOffset)) * THICKNESS;
		ClipPolygonAgainstPlane(incidentFace, -referenceNormal, -(referencePlaneOffset + k));

		for (auto& plane : edgePlanes)
			ClipPolygonAgainstPlane(incidentFace, plane.first, plane.second);

		return incidentFace;
	}
	void PhysicsSystem::ClipPolygonAgainstPlane(std::vector<glm::vec3>& points, glm::vec3 planeNormal, float planeOffset)
	{
		std::vector<glm::vec3> result;

		for (int i = 0; i < points.size(); i++)
		{
			size_t nextIndex = (i + 1) % points.size();
			glm::vec3 current = points[i];
			glm::vec3 next = points[nextIndex];

			float d1 = glm::dot(current, planeNormal) - planeOffset;
			float d2 = glm::dot(next, planeNormal) - planeOffset;

			// Case 1: Both points inside -> Keep next point
			if (d1 >= 0 && d2 >= 0)
			{
				result.push_back(next);
			}
			// Case 2: One inside, one outside -> Compute intersection and keep the inside point
			else if (d1 >= 0 && d2 < 0)
			{
				float t = d1 / (d1 - d2);
				glm::vec3 clipped = glm::mix(current, next, t);
				result.push_back(clipped);
			}
			// Case 3: One outside, one inside -> Compute intersection and keep it
			else if (d1 < 0 && d2 >= 0)
			{
				float t = d1 / (d1 - d2);
				glm::vec3 clipped = glm::mix(current, next, t);
				result.push_back(clipped);
				result.push_back(next);
			}
		}

		points = result;
	}

	bool PhysicsSystem::AreContactsValidInCache(CollisionPair colliderPair, std::vector<ContactPoint> newContacts)
	{
		static const float RESTING_CONTACT_TOLERANCE = 0.1f;

		// If there are no cached contacts, the cache must be updated
		if (cachedContacts.find(colliderPair) == cachedContacts.end())
			return false;

		std::vector<ContactPoint> contactCache = cachedContacts[colliderPair];

		// If the number of contacts is inconsistent, cache can't be valid
		if (newContacts.size() != contactCache.size())
			return false;

		// For each new contact, find the closest cached contact
		for (ContactPoint& newContact : newContacts)
		{
			ContactPoint closest;
			float closestDistance = FLT_MAX;
			for (ContactPoint& cachedContact : contactCache)
			{
				float distance = glm::distance(newContact.location, cachedContact.location);
				if (distance < closestDistance)
				{
					closest = cachedContact;
					closestDistance = distance;
				}
			}

			// If no cached point is within the tolerance range, return - use the new contacts
			if (closestDistance > RESTING_CONTACT_TOLERANCE)
				return false;
		}

		// Cache is valid if every new contact is within tolerance range to at least one cached point
		return true;
	}

	glm::vec3 PhysicsSystem::GJK_Support(const TransformComponent& transform, const ColliderComponent& collider, const glm::vec3& direction) const
	{
		glm::vec3 min = collider.center - collider.halfSize;
		glm::vec3 max = collider.center + collider.halfSize;
		std::vector<glm::vec3> vertices =
		{
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(min.x, min.y, min.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(min.x, min.y, max.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(min.x, max.y, min.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(min.x, max.y, max.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(max.x, min.y, min.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(max.x, min.y, max.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(max.x, max.y, min.z)),
			TransformUtils::LocalToWorld_Point(transform, glm::vec3(max.x, max.y, max.z))
		};

		glm::vec3 supportPoint;
		float maxDot = 0;
		for (glm::vec3 vertex : vertices)
		{
			float dot = glm::dot(direction, vertex - TransformUtils::LocalToWorld_Point(transform, collider.center));
			if (dot > maxDot)
			{
				supportPoint = vertex;
				maxDot = dot;
			}
		}

		return supportPoint;
	}
	std::vector<glm::vec3> PhysicsSystem::EPA_GetAlignedFace(const TransformComponent& transform, const ColliderComponent& collider, const glm::vec3& direction, glm::vec3& out_faceNormal) const
	{
		std::vector<glm::vec3> face;

		glm::vec3 dominant;
		glm::vec3 u;
		glm::vec3 v;
		float halfSizeDominant = 0;
		float halfSizeU = 0;
		float halfSizeV = 0;
		float dotDominant = 0;

		float dotRight = glm::dot(direction, transform.right());
		if (abs(dotRight) > abs(dotDominant))
		{
			dominant = (dotRight > 0) ? transform.right() : -transform.right();
			u = transform.forward();
			v = transform.up();

			halfSizeDominant = collider.halfSize.x;
			halfSizeU = collider.halfSize.z;
			halfSizeV = collider.halfSize.y;

			dotDominant = dotRight;
		}
		float dotUp = glm::dot(direction, transform.up());
		if (abs(dotUp) > abs(dotDominant))
		{
			dominant = (dotUp > 0) ? transform.up() : -transform.up();
			u = transform.right();
			v = transform.forward();

			halfSizeDominant = collider.halfSize.y;
			halfSizeU = collider.halfSize.x;
			halfSizeV = collider.halfSize.z;

			dotDominant = dotUp;
		}
		float dotForward = glm::dot(direction, transform.forward());
		if (abs(dotForward) > abs(dotDominant))
		{
			dominant = (dotForward > 0) ? transform.forward() : -transform.forward();
			u = -transform.right();
			v = transform.up();

			halfSizeDominant = collider.halfSize.z;
			halfSizeU = collider.halfSize.x;
			halfSizeV = collider.halfSize.y;

			dotDominant = dotForward;
		}

		glm::vec3 faceCenter = TransformUtils::LocalToWorld_Point(transform, collider.center) + dominant * halfSizeDominant;

		face.push_back(faceCenter - u * halfSizeU - v * halfSizeV);
		face.push_back(faceCenter + u * halfSizeU - v * halfSizeV);
		face.push_back(faceCenter + u * halfSizeU + v * halfSizeV);
		face.push_back(faceCenter - u * halfSizeU + v * halfSizeV);

		out_faceNormal = dominant;

		return face;
	}
}