#pragma once

#include <vector>
#include <deque>
#include <unordered_map>

#include "Vector.h"
#include "Matrix.h"
#include "PhysicsComponents.h"
#include "ECSProject/include/ecs/ECSCoordinator.hpp"

using Entity = uint32_t;

struct CollisionPair
{
private:
	const Collider* a;
	const Collider* b;

public:
	CollisionPair(const Collider* a = nullptr, const Collider* b = nullptr) : a(a), b(b) {}

	bool operator==(const CollisionPair& other) const
	{
		// Order-agnostic equality
		return (&a == &other.a && &b == &other.b)
			|| (&a == &other.b && &b == &other.a);
	}

	const Collider* GetFirst() const { return a; };
	const Collider* GetSecond() const { return b; };

	friend std::hash<CollisionPair>;
};
namespace std
{
	template<>
	struct hash<CollisionPair>
	{
		std::size_t operator()(CollisionPair collisionPair) const
		{
			/* Hash and combine pointers */

			uintptr_t hashA = reinterpret_cast<uintptr_t>(&collisionPair.a);
			uintptr_t hashB = reinterpret_cast<uintptr_t>(&collisionPair.b);

			// Sort for order-independence
			if(hashA > hashB)
				std::swap(hashA, hashB);

			return hashA ^ (hashB << 1);
		}
	};
}

struct ContactPoint
{
	Vector location;
	Vector normal;
	float penetrationDepth;
};

struct CollisionData
{
	Entity entityA;
	Entity entityB;
	CollisionPair colliderPair;
	std::vector<ContactPoint> contacts;
};

class PhysicsSystem
{
private:
	std::vector<Rigidbody*> rigidbodies;

	std::unordered_map<CollisionPair, std::vector<ContactPoint>> cachedContacts;

	bool tickPhysics = true;

public:
	PhysicsSystem();
	~PhysicsSystem();

	static void Startup();
	static void Shutdown();

	static void Update(ECSCoordinator& ecs, float deltaTime);

	static void Pause();
	static void Unpause();

	static void AddRigidbody(Rigidbody* rigidbody);

private:
	bool GJK(const Collider& a, const Collider& b, std::vector<ContactPoint>& out_contactPoints);
	bool UpdateSimplex(std::deque<Vector>& simplex, Vector& direction);

	bool UpdateSimplex_LineCase(std::deque<Vector>& simplex, Vector& direction);
	bool UpdateSimplex_TriangleCase(std::deque<Vector>& simplex, Vector& direction);
	bool UpdateSimplex_TetrahedronCase(std::deque<Vector>& simplex, Vector& direction);

	std::vector<ContactPoint> EPA(std::deque<Vector> simplex, std::unordered_map<Vector, std::pair<Vector, Vector>> supportPoints, const Collider& a, const Collider& b);
	void EPA_GetPolytopeNormals(std::vector<Vector> polytope, std::vector<unsigned int> faces, std::vector<std::pair<Vector, float>>& out_normals, unsigned int& out_minNormalIndex);
	void EPA_AddIfUniqueEdge(std::vector<std::pair<unsigned int, unsigned int>>& edges, const std::vector<unsigned int>& faces, unsigned int a, unsigned int b);

	std::vector<Vector> GetContactPoints(std::vector<Vector> incidentFace, std::vector<Vector> referenceFace, Vector referenceNormal);
	void ClipPolygonAgainstPlane(std::vector<Vector>& points, Vector planeNormal, float planeOffset);
	std::vector<Vector> ClipPolygonAgainstEdge(std::vector<Vector>& points, Vector planeNormal);

	bool AreContactsValidInCache(CollisionPair colliderPair, std::vector<ContactPoint> newContacts);

	/* Component helpers */

	// Convex colliders
	Vector GJK_Support(const Collider& collider, const Vector& direction) const;
	std::vector<Vector> EPA_GetAlignedFace(const Collider& collider, const Vector& direction, Vector& out_faceNormal) const;
};