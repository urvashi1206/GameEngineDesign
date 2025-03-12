#pragma once

#include <vector>
#include <deque>

#include "Rigidbody.h"
#include "BoxCollider.h"

struct CollisionPair
{
private:
	const ConvexCollider* a;
	const ConvexCollider* b;

public:
	CollisionPair() : a(nullptr), b(nullptr)
	{

	}
	CollisionPair(const ConvexCollider* a, const ConvexCollider* b) : a(a), b(b)
	{
		
	}

	bool operator==(const CollisionPair& other) const
	{
		// Order-agnostic equality
		return a == other.a && b == other.b
			|| a == other.b && b == other.a;
	}

	const ConvexCollider* GetFirst() const { return a; };
	const ConvexCollider* GetSecond() const { return b; };

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

			uintptr_t hashA = reinterpret_cast<uintptr_t>(collisionPair.a);
			uintptr_t hashB = reinterpret_cast<uintptr_t>(collisionPair.b);

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
	Rigidbody* bodyA;
	Rigidbody* bodyB;
	CollisionPair colliderPair;
	std::vector<ContactPoint> contacts;

	CollisionData() : bodyA(), bodyB(), contacts()
	{

	}
};

class PhysicsSubsystem
{
private:
	std::vector<Rigidbody> rigidbodies;

	std::unordered_map<CollisionPair, std::vector<ContactPoint>> cachedContacts;

	bool tickPhysics = true;

public:
	PhysicsSubsystem();
	~PhysicsSubsystem();

	void Update(float deltaTime);

	void Pause();
	void Unpause();

	void AddRigidbody(Rigidbody rigidbody);

	void TestCollision_Box_Box(const BoxCollider* a, const BoxCollider* b, bool& out_colliding, std::vector<ContactPoint>& out_contactPoints);

	bool GJK(const ConvexCollider* a, const ConvexCollider* b, std::vector<ContactPoint>& out_contactPoints);
	bool UpdateSimplex(std::deque<Vector>& simplex, Vector& direction);

	bool UpdateSimplex_LineCase(std::deque<Vector>& simplex, Vector& direction);
	bool UpdateSimplex_TriangleCase(std::deque<Vector>& simplex, Vector& direction);
	bool UpdateSimplex_TetrahedronCase(std::deque<Vector>& simplex, Vector& direction);

	std::vector<ContactPoint> EPA(std::deque<Vector> simplex, std::unordered_map<Vector, std::pair<Vector, Vector>> supportPoints, const ConvexCollider* a, const ConvexCollider* b);
	void EPA_GetPolytopeNormals(std::vector<Vector> polytope, std::vector<unsigned int> faces, std::vector<std::pair<Vector, float>>& out_normals, unsigned int& out_minNormalIndex);
	void EPA_AddIfUniqueEdge(std::vector<std::pair<unsigned int, unsigned int>>& edges, const std::vector<unsigned int>& faces, unsigned int a, unsigned int b);

	std::vector<Vector> GetContactPoints(std::vector<Vector> incidentFace, std::vector<Vector> referenceFace, Vector referenceNormal);
	void ClipPolygonAgainstPlane(std::vector<Vector>& points, Vector planeNormal, float planeOffset);
	std::vector<Vector> ClipPolygonAgainstEdge(std::vector<Vector>& points, Vector planeNormal);

	bool AreContactsValidInCache(CollisionPair colliderPair, std::vector<ContactPoint> newContacts);

	Rigidbody GetRigidbody(unsigned int index) { return rigidbodies[index]; };
};