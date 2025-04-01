#include "PhysicsSubsystem.h"

#include "ConvexCollider.h"
#include "CompositeCollider.h"

#include <iostream>

#include "Debug.h"

namespace
{
	PhysicsSubsystem* Instance;
}

PhysicsSubsystem::PhysicsSubsystem()
{

}
PhysicsSubsystem::~PhysicsSubsystem()
{

}

void PhysicsSubsystem::Startup()
{
	Instance = new PhysicsSubsystem();
}
void PhysicsSubsystem::Shutdown()
{
	delete Instance;
}

void PhysicsSubsystem::Update(float deltaTime)
{
	static const float PHYSICS_TICK = 0.01f;

	static float simulationTimeLeft = 0;

	simulationTimeLeft += deltaTime;

	while(simulationTimeLeft >= PHYSICS_TICK)
	{
		std::vector<CollisionData> collisions;

		/* Physics Update */
	
		for(Rigidbody* rb : Instance->rigidbodies)
		{
			rb->ApplyGravity();

			rb->UpdatePhysics(Instance->tickPhysics ? PHYSICS_TICK : 0);
		}

		/* Collision Detection and Contact Generation */

		for(size_t i = 0; i < Instance->rigidbodies.size(); i++)
			for(size_t j = 0; j < Instance->rigidbodies.size(); j++)
			{
				if(i == j)
					continue;

				// Colliders involved in this pair to check collision for
				std::vector<ConvexCollider*> convexCollidersA;
				std::vector<ConvexCollider*> convexCollidersB;

				switch(Instance->rigidbodies[i]->GetCollider()->GetColliderType())
				{
					case ColliderType::None: break;
					case ColliderType::Convex:
						convexCollidersA.push_back((ConvexCollider*) Instance->rigidbodies[i]->GetCollider());
						break;
					case ColliderType::Composite:
						for(ConvexCollider* c : ((CompositeCollider*) Instance->rigidbodies[i]->GetCollider())->GetColliders())
							convexCollidersA.push_back(c);
						break;
					default: break;
				}
				switch(Instance->rigidbodies[j]->GetCollider()->GetColliderType())
				{
					case ColliderType::None: break;
					case ColliderType::Convex:
						convexCollidersB.push_back((ConvexCollider*) Instance->rigidbodies[j]->GetCollider());
						break;
					case ColliderType::Composite:
						for(ConvexCollider* c : ((CompositeCollider*) Instance->rigidbodies[j]->GetCollider())->GetColliders())
							convexCollidersB.push_back(c);
						break;
					default: break;
				}

				for(ConvexCollider* a : convexCollidersA)
					for(ConvexCollider* b : convexCollidersB)
					{
						std::vector<ContactPoint> contactPoints;
						if(Instance->GJK(a, b, contactPoints))
						{
							CollisionData collisionData {};
							collisionData.bodyA = Instance->rigidbodies[i];
							collisionData.bodyB = Instance->rigidbodies[j];
							collisionData.colliderPair = CollisionPair(a, b);
							collisionData.contacts = contactPoints;

							collisions.push_back(collisionData);
						}
					}
			}

		/* Collision Resolution */

		for(CollisionData& collisionData : collisions)
		{
			Rigidbody* bodyA = collisionData.bodyA;
			Rigidbody* bodyB = collisionData.bodyB;

			Transform* transformA = bodyA->GetTransform();
			Transform* transformB = bodyB->GetTransform();

			Vector initialLocationA = collisionData.colliderPair.GetFirst()->GetCenter();
			Vector initialLocationB = collisionData.colliderPair.GetSecond()->GetCenter();
			Vector initialVelocityA = bodyA->GetVelocity();
			Vector initialVelocityB = bodyB->GetVelocity();
			Vector initialAngularVelocityA = bodyA->GetAngularVelocity();
			Vector initialAngularVelocityB = bodyB->GetAngularVelocity();

			for(const ContactPoint& contact : collisionData.contacts)
			{
				Debug::CreateWireframe_Temp(contact.location, Vector(), Vector(0.1f, 0.1f, 0.1f));

				// Location and velocity relative to center of mass
				Vector rA = contact.location - collisionData.colliderPair.GetFirst()->GetCenter();
				Vector rB = contact.location - collisionData.colliderPair.GetSecond()->GetCenter();
				Vector relativeVelocity = bodyB->GetVelocity() + bodyB->GetAngularVelocity().Cross(rB) - bodyA->GetVelocity() - bodyA->GetAngularVelocity().Cross(rA);

				// Calculate relative velocity along the normal
				float velocityAlongNormal = relativeVelocity.Dot(contact.normal);

				// If the velocities are separating, no need to resolve
				if(velocityAlongNormal > 0)
					continue;

				// Calculate restitution (bounciness)
				float e = min(bodyA->bounciness, bodyB->bounciness);

				// Calculate impulse scalar
				float j = -(1 + e) * velocityAlongNormal;
				j /= bodyA->GetInverseMass() + bodyB->GetInverseMass() +
					contact.normal.Dot((bodyA->GetInertiaTensor().Inverse() * rA.Cross(contact.normal)).Cross(rA)) +
					contact.normal.Dot((bodyB->GetInertiaTensor().Inverse() * rB.Cross(contact.normal)).Cross(rB));

				// Apply impulse
				Vector impulse = contact.normal * j;
				bodyA->ApplyImpulse(-impulse, contact.location);
				bodyB->ApplyImpulse(impulse, contact.location);

				/* Friction */
				
				// Use original location and velocity values for friction
				rA = contact.location - initialLocationA;
				rB = contact.location - initialLocationB;
				relativeVelocity = initialVelocityB + initialAngularVelocityB.Cross(rB) - initialVelocityA - initialAngularVelocityA.Cross(rA);

				// Determine tangential velocity
				Vector tangentialVelocity = relativeVelocity - contact.normal * velocityAlongNormal;
				if(tangentialVelocity.GetSqrMagnitude() > 1e-6f)
					tangentialVelocity = tangentialVelocity.Normalized();
				else
					tangentialVelocity = Vector::Zero();

				// Determine whether to use static or dynamic friction and scale max impulse
				float coefficient = tangentialVelocity == Vector::Zero() ? 
					sqrt(pow(bodyA->GetStaticFriction(), 2) + pow(bodyB->GetStaticFriction(), 2)) 
					: sqrt(pow(bodyA->GetDynamicFriction(), 2) + pow(bodyB->GetDynamicFriction(), 2));
				float maxFrictionImpulse = coefficient * j;

				Vector rA_cross_t = rA.Cross(tangentialVelocity);
				Vector rB_cross_t = rB.Cross(tangentialVelocity);
				float angularEffectFrictionA = rA_cross_t.Dot(bodyA->GetInertiaTensor().Inverse() * rA_cross_t);
				float angularEffectFrictionB = rB_cross_t.Dot(bodyB->GetInertiaTensor().Inverse() * rB_cross_t);

				float frictionInertia = bodyA->GetInverseMass() + bodyB->GetInverseMass() + angularEffectFrictionA + angularEffectFrictionB;
				float frictionImpulseMagnitude = -relativeVelocity.Dot(tangentialVelocity) / frictionInertia;
				frictionImpulseMagnitude = std::clamp(frictionImpulseMagnitude, -maxFrictionImpulse, maxFrictionImpulse);

				Vector frictionImpulse = tangentialVelocity * frictionImpulseMagnitude;

				// Apply friction impulse
				bodyA->ApplyImpulse(-frictionImpulse, contact.location);
				bodyB->ApplyImpulse(frictionImpulse, contact.location);

				// Positional correction to prevent sinking
				const float percent = 0.2f; // Usually 20% to 80%
				const float slop = 0.01f; // Usually 0.01 to 0.1
				Vector correction = contact.normal * max(contact.penetrationDepth - slop, 0.0f) / (bodyA->GetInverseMass() + bodyB->GetInverseMass()) * percent;
				transformA->MoveAbsolute(-correction * bodyA->GetInverseMass());
				transformB->MoveAbsolute(correction * bodyB->GetInverseMass());
			}

			//Pause();

			//break;
		}

		simulationTimeLeft -= PHYSICS_TICK;
	}
}

void PhysicsSubsystem::Pause()
{
	Instance->tickPhysics = false;
}
void PhysicsSubsystem::Unpause()
{
	Instance->tickPhysics = true;
}

void PhysicsSubsystem::AddRigidbody(Rigidbody* rigidbody)
{
	Instance->rigidbodies.push_back(rigidbody);
}

void PhysicsSubsystem::TestCollision_Box_Box(const BoxCollider* a, const BoxCollider* b, bool& out_colliding, std::vector<ContactPoint>& out_contactPoints)
{
	out_colliding = false;
	
	/*out_colliding = a->transform->GetLocation().x > b->GetWorldMin().x && a->transform->GetLocation().x < b->GetWorldMax().x
		&& a->transform->GetLocation().y > b->GetWorldMin().y && a->transform->GetLocation().y < b->GetWorldMax().y
		&& a->transform->GetLocation().z > b->GetWorldMin().z && a->transform->GetLocation().z < b->GetWorldMax().z;*/

	out_colliding = GJK(a, b, out_contactPoints);
	//assert(GJK(a, b) == GJK(b, a));
}

bool PhysicsSubsystem::GJK(const ConvexCollider* a, const ConvexCollider* b, std::vector<ContactPoint>& out_contactPoints)
{
	// Arbitrary direction as a starting point
	Vector direction(1, 0, 0);
	//assert((a->GJK_Support(direction) - b->GJK_Support(-direction)) == -(b->GJK_Support(direction) - a->GJK_Support(-direction)));
	//assert();
	// Get the Minkowski Difference on the hull in this direction
	Vector difference = a->GJK_Support(direction) - b->GJK_Support(-direction);
	// Initialize the simplex
	std::deque<Vector> simplex { difference };
	std::unordered_map<Vector, std::pair<Vector, Vector>> supportPoints;

	// Get the new direction towards the origin
	direction = -simplex.back();

	// Add up to three more points to the simplex to try to contain the origin
	while(true)
	{
		// Get the next Minkowski Difference on the hull
		Vector support = a->GJK_Support(direction) - b->GJK_Support(-direction);
		// If we didn't pass the origin, return false
		if(support.Dot(direction) <= 0)
			return false;

		// Add new point to simplex
		simplex.push_front(support);
		supportPoints.emplace(support, std::make_pair<Vector, Vector>(a->GJK_Support(direction), b->GJK_Support(-direction)));

		// Test for the simplex intersecting the origin
		if(UpdateSimplex(simplex, direction))
		{
			// Perform EPA to determine collision normal
			out_contactPoints = EPA(simplex, supportPoints, a, b);

			return true;
		}
	}
}
bool PhysicsSubsystem::UpdateSimplex(std::deque<Vector>& simplex, Vector& direction)
{
	switch(simplex.size())
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

bool PhysicsSubsystem::UpdateSimplex_LineCase(std::deque<Vector>& simplex, Vector& direction)
{
	Vector a = simplex[0];
	Vector b = simplex[1];

	Vector ao = -a;
	Vector ab = b - a; // The only edge

	if(ab.Dot(ao) > 0)
		direction = ab.Cross(ao).Cross(ab);
	else
	{
		simplex = { a };
		direction = ao;
	}

	return false;
}
bool PhysicsSubsystem::UpdateSimplex_TriangleCase(std::deque<Vector>& simplex, Vector& direction)
{
	Vector a = simplex[0];
	Vector b = simplex[1];
	Vector c = simplex[2];

	Vector ao = -a;
	Vector ab = b - a; // Edge 1
	Vector ac = c - a; // Edge 2

	Vector abc = ab.Cross(ac); // Normal of the triangle

	if((abc.Cross(ac)).Dot(ao) > 0)
	{
		if(ac.Dot(ao) > 0)
		{
			simplex = { a, c };
			direction = ac.Cross(ao).Cross(ac);
		}
		else
			UpdateSimplex_LineCase(simplex = { a, b }, direction);
	}
	else
	{
		if((ab.Cross(abc)).Dot(ao) > 0)
			return UpdateSimplex_LineCase(simplex = { a, b }, direction);
		else
		{
			if(abc.Dot(ao) > 0)
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
bool PhysicsSubsystem::UpdateSimplex_TetrahedronCase(std::deque<Vector>& simplex, Vector& direction)
{
	Vector a = simplex[0];
	Vector b = simplex[1];
	Vector c = simplex[2];
	Vector d = simplex[3];

	Vector ao = -a;
	Vector ab = b - a;
	Vector ac = c - a;
	Vector ad = d - a;

	Vector abc = ab.Cross(ac);
	Vector acd = ac.Cross(ad);
	Vector adb = ad.Cross(ab);

	if(abc.Dot(ao) > 0)
		return UpdateSimplex_TriangleCase(simplex = { a, b, c }, direction);
	if(acd.Dot(ao) > 0)
		return UpdateSimplex_TriangleCase(simplex = { a, c, d }, direction);
	if(adb.Dot(ao) > 0)
		return UpdateSimplex_TriangleCase(simplex = { a, d, b }, direction);

	return true;
}

std::vector<ContactPoint> PhysicsSubsystem::EPA(std::deque<Vector> simplex, std::unordered_map<Vector, std::pair<Vector, Vector>> supportPoints, const ConvexCollider* a, const ConvexCollider* b)
{
	// The acceptable range for a point to be considered on the boundary of the Minkowski Difference
	static const float EDGE_TOLERANCE = 0.01f;

	// The polytope, represented by a series of vertices.
	std::vector<Vector> polytope(simplex.begin(), simplex.end());
	// The faces of the polytope, represented by index triplets indicating the vertices in each face.
	std::vector<unsigned int> faces =
	{
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	// Normals and distances
	std::vector<std::pair<Vector, float>> normals;
	unsigned int minNormalIndex;
	EPA_GetPolytopeNormals(polytope, faces, normals, minNormalIndex);

	Vector minNormal;
	float minDistance = FLT_MAX;
	while(minDistance == FLT_MAX)
	{
		minNormal = normals[minNormalIndex].first;
		minDistance = normals[minNormalIndex].second;

		Vector supportPoint = a->GJK_Support(minNormal) - b->GJK_Support(-minNormal);
		float sDistance = minNormal.Dot(supportPoint);

		// If the polytope already contains the support point, terminate - we have found the closest face
		if(std::find(polytope.begin(), polytope.end(), supportPoint) != polytope.end())
			break;

		if(abs(sDistance - minDistance) > EDGE_TOLERANCE)
		{
			minDistance = FLT_MAX;

			std::vector<std::pair<unsigned int, unsigned int>> uniqueEdges;
			for(unsigned int i = 0; i < normals.size(); i++)
			{
				if(normals[i].first.Dot(supportPoint) > 0)
				{
					unsigned int f = i * 3;

					EPA_AddIfUniqueEdge(uniqueEdges, faces, f,		f + 1);
					EPA_AddIfUniqueEdge(uniqueEdges, faces, f + 1,	f + 2);
					EPA_AddIfUniqueEdge(uniqueEdges, faces, f + 2,	f);

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
			for(auto [edgeIndex1, edgeIndex2] : uniqueEdges)
			{
				newFaces.push_back(edgeIndex1);
				newFaces.push_back(edgeIndex2);
				newFaces.push_back(polytope.size());
			}
			
			polytope.push_back(supportPoint);
			supportPoints.emplace(supportPoint, std::make_pair<Vector, Vector>(a->GJK_Support(minNormal), b->GJK_Support(-minNormal)));
			
			std::vector<std::pair<Vector, float>> newNormals;
			unsigned int newMinFace;
			EPA_GetPolytopeNormals(polytope, newFaces, newNormals, newMinFace);

			float oldMinDistance = FLT_MAX;
			for(size_t i = 0; i < normals.size(); i++)
			{
				if(normals[i].second < oldMinDistance)
				{
					oldMinDistance = normals[i].second;
					minNormalIndex = i;
				}
			}

			if(newNormals[newMinFace].second < oldMinDistance)
				minNormalIndex = newMinFace + normals.size();

			faces.insert(faces.end(), newFaces.begin(), newFaces.end());
			normals.insert(normals.end(), newNormals.begin(), newNormals.end());
		}
	}

	std::vector<ContactPoint> contactPoints;

	/* Generate contact points */

	// Points on the closest face of the polytope
	Vector A = polytope[faces[minNormalIndex * 3]];
	Vector B = polytope[faces[minNormalIndex * 3 + 1]];
	Vector C = polytope[faces[minNormalIndex * 3 + 2]];

	// Project origin onto contact plane
	Vector P = Vector().ProjectOntoPlane(A, minNormal);

	// The corresponding support points on the original colliders
	Vector Aa = supportPoints[A].first;
	Vector Ab = supportPoints[B].first;
	Vector Ac = supportPoints[C].first;

	Vector Ba = supportPoints[A].second;
	Vector Bb = supportPoints[B].second;
	Vector Bc = supportPoints[C].second;

	// Areas of the triangles in the face with the projected origin (for calculating barycentrics)
	float areaPBC = ((B - P).Cross(C - P)).Dot((B - A).Cross(C - A).Normalized());
	float areaPCA = ((C - P).Cross(A - P)).Dot((B - A).Cross(C - A).Normalized());
	float areaABC = ((B - A).Cross(C - A)).Dot((B - A).Cross(C - A).Normalized());

	// Barycentric coordinates
	float alpha = areaPBC / areaABC;
	float beta = areaPCA / areaABC;
	float gamma = 1 - alpha - beta;

	Vector l = A * alpha + B * beta + C * gamma;
	//assert((l - P).GetMagnitude() < 0.1f);

	// Contact point on collider A
	Vector Ap = Aa * alpha + Ab * beta + Ac * gamma;
	// Contact point on collider B
	Vector Bp = Ba * alpha + Bb * beta + Bc * gamma;

	//Debug::CreateWireframe_Temp(Ap, Vector(), Vector(0.25f, 0.25f, 0.25f));
	//Debug::CreateWireframe_Temp(Bp, Vector(), Vector(0.25f, 0.25f, 0.25f));
	//Debug::CreateWireframe_Temp((Ap + Bp) / 2, Vector(), Vector(0.25f, 0.25f, 0.25f)); // The midpoint between the contact points on each collider is the final contact point

	//Debug::CreateWireframe_Temp(Aa, Vector(), Vector(0.125f, 0.125f, 0.125f));
	//Debug::CreateWireframe_Temp(Ab, Vector(), Vector(0.125f, 0.125f, 0.125f));
	//Debug::CreateWireframe_Temp(Ac, Vector(), Vector(0.125f, 0.125f, 0.125f));
	//Debug::CreateWireframe_Temp(Ap, Vector(), Vector(0.125f, 0.125f, 0.125f));
	//Debug::CreateWireframe_Temp(P, Vector(), Vector(0.125f, 0.125f, 0.125f));
	//Debug::CreateWireframe_Temp(l, Vector(), Vector(0.125f, 0.125f, 0.125f));

	// For now, only return the contact point for collider A
	ContactPoint contactPoint;
	contactPoint.location = (Ap + Bp) / 2;
	contactPoint.normal = minNormal;
	contactPoint.penetrationDepth = minDistance + EDGE_TOLERANCE;

	//contactPoints.push_back(contactPoint);

	Vector normalA;
	std::vector<Vector> faceA = a->EPA_GetAlignedFace(minNormal, normalA);
	Vector normalB;
	std::vector<Vector> faceB = b->EPA_GetAlignedFace(-minNormal, normalB);

	std::vector<Vector> clipAB = GetContactPoints(faceA, faceB, normalB);
	std::vector<Vector> clipBA = GetContactPoints(faceB, faceA, normalA);

	std::vector<Vector> contacts;
	for(Vector& p : clipAB)
		contacts.push_back(p);
	for(Vector& p : clipBA)
		contacts.push_back(p);

	for(Vector point : clipAB)
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
	CollisionPair colliderPair(a, b);
	if(AreContactsValidInCache(colliderPair, contactPoints))
		return cachedContacts[colliderPair];

	// Cache the contacts between these two colliders so they aren't regenerated if there is no significant difference in locations ("resting contact")
	cachedContacts[colliderPair] = contactPoints;

	return contactPoints;
}
void PhysicsSubsystem::EPA_GetPolytopeNormals(std::vector<Vector> polytope, std::vector<unsigned int> faces, std::vector<std::pair<Vector, float>>& out_normals, unsigned int& out_minNormalIndex)
{
	// Initial index of the minimum distance triangle
	unsigned int minTriangleIndex = 0;
	float minDistance = FLT_MAX;

	for(unsigned int i = 0; i < faces.size(); i += 3)
	{
		Vector a = polytope[faces[i]];
		Vector b = polytope[faces[i + 1]];
		Vector c = polytope[faces[i + 2]];

		Vector normal = (b - a).Cross(c - a).Normalized();
		float distance = normal.Dot(a);
		// Reverse normal if facing inward
		if(distance < 0)
		{
			normal = -normal;
			distance = -distance;
		}

		// Update out variables
		out_normals.emplace_back(normal, distance);

		if(distance < minDistance)
		{
			minTriangleIndex = i / 3;
			minDistance = distance;
		}
	}

	out_minNormalIndex = minTriangleIndex;
}
void PhysicsSubsystem::EPA_AddIfUniqueEdge(std::vector<std::pair<unsigned int, unsigned int>>& edges, const std::vector<unsigned int>& faces, unsigned int a, unsigned int b)
{
	auto reverse = std::find(edges.begin(), edges.end(), std::make_pair(faces[b], faces[a]));

	if(reverse != edges.end())
		edges.erase(reverse);
	else
		edges.emplace_back(faces[a], faces[b]);
}

std::vector<Vector> PhysicsSubsystem::GetContactPoints(std::vector<Vector> incidentFace, std::vector<Vector> referenceFace, Vector referenceNormal)
{
	static const float THICKNESS = 0.001f;

	if(incidentFace.size() == 1)
		return incidentFace;
	if(referenceFace.size() == 1)
		return referenceFace;

	assert(incidentFace.size() > 2 && referenceFace.size() > 2);

	Vector incidentNormal = (incidentFace[1] - incidentFace[0]).Cross(incidentFace[2] - incidentFace[0]).Normalized();

	Vector referenceCenter = Vector::Zero();
	for(int i = 0; i < referenceFace.size(); i++)
		referenceCenter += referenceFace[i];
	referenceCenter /= referenceFace.size();

	std::unordered_map<Vector, float> edgePlanes;
	for(size_t i = 0; i < referenceFace.size(); i++)
	{
		Vector currentVertex = referenceFace[i];
		Vector nextVertex = referenceFace[(i + 1) % referenceFace.size()];

		Vector edgeNormal = (nextVertex - currentVertex).Cross(referenceNormal).Normalized();
		float planeOffset = edgeNormal.Dot(currentVertex);
		if(edgeNormal.Dot(currentVertex - referenceCenter) > 0)
		{
			edgeNormal = -edgeNormal;
			planeOffset = -planeOffset;
		}
		edgePlanes.emplace(edgeNormal, planeOffset);
	}

	// All edge normals should be unique for convex shapes; if not, vertex order is incorrect
	assert(edgePlanes.size() == referenceFace.size());

	float referencePlaneOffset = referenceNormal.Dot(referenceFace[0]);
	float k = referencePlaneOffset / abs(referencePlaneOffset) * THICKNESS;
	ClipPolygonAgainstPlane(incidentFace, -referenceNormal, -(referencePlaneOffset + k));

	for(auto& plane : edgePlanes)
		ClipPolygonAgainstPlane(incidentFace, plane.first, plane.second);

	return incidentFace;
}
void PhysicsSubsystem::ClipPolygonAgainstPlane(std::vector<Vector>& points, Vector planeNormal, float planeOffset)
{
	std::vector<Vector> result;

	for(int i = 0; i < points.size(); i++)
	{
		size_t nextIndex = (i + 1) % points.size();
		Vector current = points[i];
		Vector next = points[nextIndex];

		float d1 = current.Dot(planeNormal) - planeOffset;
		float d2 = next.Dot(planeNormal) - planeOffset;

		// Case 1: Both points inside -> Keep next point
        if(d1 >= 0 && d2 >= 0)
		{
			result.push_back(next);
        }
        // Case 2: One inside, one outside -> Compute intersection and keep the inside point
        else if(d1 >= 0 && d2 < 0)
		{
            float t = d1 / (d1 - d2);
            Vector clipped = Vector::Lerp(current, next, t);
			result.push_back(clipped);
        }
        // Case 3: One outside, one inside -> Compute intersection and keep it
        else if(d1 < 0 && d2 >= 0)
		{
            float t = d1 / (d1 - d2);
			Vector clipped = Vector::Lerp(current, next, t);
			result.push_back(clipped);
			result.push_back(next);
        }
	}

	points = result;
}
std::vector<Vector> PhysicsSubsystem::ClipPolygonAgainstEdge(std::vector<Vector>& points, Vector normal)
{
	std::vector<Vector> clipped;

	for(int i = 0; i < points.size(); i++)
	{
		Vector currentVertex = points[i];
		Vector nextVertex = points[(i + 1) % points.size()];

		bool currentInside = normal.Dot(currentVertex) >= 0;
		bool nextInside = normal.Dot(currentVertex) >= 0;

		if(currentInside && !nextInside)
			//
		if(!currentInside && nextInside)
			//
		if(currentInside && nextInside)
			clipped.push_back(nextVertex);
	}

	return clipped;
}

bool PhysicsSubsystem::AreContactsValidInCache(CollisionPair colliderPair, std::vector<ContactPoint> newContacts)
{
	static const float RESTING_CONTACT_TOLERANCE = 0.1f;

	// If there are no cached contacts, the cache must be updated
	if(cachedContacts.find(colliderPair) == cachedContacts.end())
		return false;

	std::vector<ContactPoint> contactCache = cachedContacts[colliderPair];
	
	// If the number of contacts is inconsistent, cache can't be valid
	if(newContacts.size() != contactCache.size())
		return false;
	
	// For each new contact, find the closest cached contact
	for(ContactPoint& newContact : newContacts)
	{
		ContactPoint closest;
		float closestDistance = FLT_MAX;
		for(ContactPoint& cachedContact : contactCache)
		{
			float distance = newContact.location.Distance(cachedContact.location);
			if(distance < closestDistance)
			{
				closest = cachedContact;
				closestDistance = distance;
			}
		}

		// If no cached point is within the tolerance range, return - use the new contacts
		if(closestDistance > RESTING_CONTACT_TOLERANCE)
			return false;
	}

	// Cache is valid if every new contact is within tolerance range to at least one cached point
	return true;
}