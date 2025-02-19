#pragma once

#include <vector>
#include <deque>

#include "Rigidbody.h"
#include "BoxCollider.h"

class PhysicsSubsystem
{
private:
	std::vector<Rigidbody> rigidbodies;

public:
	PhysicsSubsystem();
	~PhysicsSubsystem();

	void Update(float deltaTime);

	void AddRigidbody(Rigidbody rigidbody);

	void TestCollision_Box_Box(const BoxCollider* a, const BoxCollider* b, bool& out_colliding, Vector& out_normal);

	bool GJK(const ConvexCollider* a, const ConvexCollider* b, Vector& out_normal);
	bool UpdateSimplex(std::deque<Vector>& simplex, Vector& direction);

	bool UpdateSimplex_LineCase(std::deque<Vector>& simplex, Vector& direction);
	bool UpdateSimplex_TriangleCase(std::deque<Vector>& simplex, Vector& direction);
	bool UpdateSimplex_TetrahedronCase(std::deque<Vector>& simplex, Vector& direction);

	Rigidbody GetRigidbody(unsigned int index) { return rigidbodies[index]; };
};