#include "PhysicsSubsystem.h"

#include "BoxCollider.h"

#include <iostream>

#include "Debug.h"

PhysicsSubsystem::PhysicsSubsystem()
{

}
PhysicsSubsystem::~PhysicsSubsystem()
{

}

void PhysicsSubsystem::Update(float deltaTime)
{
	int i = -1;
	for(Rigidbody& rb : rigidbodies)
	{
		i++;
		//rb.ApplyGravity();

		//rb.AddTorque(Vector(1, 0, 0), 1);

		rb.UpdatePhysics(deltaTime);

		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(-1, -1, -1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(-1, -1, 1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(-1, 1, -1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(-1, 1, 1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(1, -1, -1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(1, -1, 1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(1, 1, -1)), Vector(), Vector(0.25f, 0.25f, 0.25f));
		Debug::CreateWireframe_Temp(((ConvexCollider*) rb.GetCollider())->GJK_Support(Vector(1, 1, 1)), Vector(), Vector(0.25f, 0.25f, 0.25f));

		int j = -1;
		for(Rigidbody& other : rigidbodies)
		{
			j++;
			Transform* transform = rb.GetTransform();
			Transform* otherTransform = other.GetTransform();

			if(otherTransform == transform)
				continue;

			if(!rb.isStatic)
				int k = 0;

			float restitution = sqrt(rb.bounciness * other.bounciness);

			bool areColliding;
			Vector collisionNormal;
			TestCollision_Box_Box((BoxCollider*) rb.GetCollider(), (BoxCollider*) other.GetCollider(), areColliding, collisionNormal);
			/*switch((int) rb.GetCollider()->GetColliderType() | (int) other.GetCollider()->GetColliderType())
			{
				case (int) ColliderType::Box:
					TestCollision_Box_Box((BoxCollider*) rb.GetCollider(), (BoxCollider*) other.GetCollider(), areColliding, collisionNormal);
					break;
			}*/

			/*if(transform->GetLocation().x > other.GetMin().x && transform->GetLocation().x < other.GetMax().x
				&& transform->GetLocation().y > other.GetMin().y && transform->GetLocation().y < other.GetMax().y
				&& transform->GetLocation().z > other.GetMin().z && transform->GetLocation().z < other.GetMax().z)*/

			std::cout << i << "," << j << " " << (areColliding ? "!" : "?") << std::endl;

			if(areColliding)
			{
				//if(!rb.isStatic)
				{
					Vector diff(
						transform->GetLocation().x - otherTransform->GetLocation().x,
						transform->GetLocation().y - otherTransform->GetLocation().y,
						transform->GetLocation().z - otherTransform->GetLocation().z);
					//rb.AddForce(diff);

					diff /= 80;

					float impulse = (-(1 + restitution) * (rb.GetVelocity() - other.GetVelocity()).Dot(collisionNormal)) / ((1.0f / rb.GetMass()) + (1.0f / other.GetMass()));

					Vector targetVelocity = rb.GetVelocity() + collisionNormal * (impulse / rb.GetMass());
					Vector otherVelocity = other.GetVelocity() - collisionNormal * (impulse / other.GetMass());

					//transform->MoveAbsolute(diff.x, diff.y, diff.z);
					rb.SetVelocity(targetVelocity);
					other.SetVelocity(otherVelocity);
				}
				//if(!other.isStatic)
				//{
				//	Vector diff(
				//		otherTransform->GetLocation().x - transform->GetLocation().x,
				//		otherTransform->GetLocation().y - transform->GetLocation().y,
				//		otherTransform->GetLocation().z - transform->GetLocation().z);
				//	//other.AddForce(diff);

				//	diff /= 2;

				//	otherTransform->MoveAbsolute(diff.x, diff.y, diff.z);
				//	rb.SetVelocity(Vector());
				//}
			}
		}

		//for()
	}
}

void PhysicsSubsystem::AddRigidbody(Rigidbody rigidbody)
{
	rigidbodies.push_back(rigidbody);
}

void PhysicsSubsystem::TestCollision_Box_Box(const BoxCollider* a, const BoxCollider* b, bool& out_colliding, Vector& out_normal)
{
	out_colliding = false;
	
	/*out_colliding = a->transform->GetLocation().x > b->GetWorldMin().x && a->transform->GetLocation().x < b->GetWorldMax().x
		&& a->transform->GetLocation().y > b->GetWorldMin().y && a->transform->GetLocation().y < b->GetWorldMax().y
		&& a->transform->GetLocation().z > b->GetWorldMin().z && a->transform->GetLocation().z < b->GetWorldMax().z;*/

	out_colliding = GJK(a, b, out_normal);
	//assert(GJK(a, b) == GJK(b, a));

	/*std::cout << "Pos: " << b->transform->GetLocation().x << " " << b->transform->GetLocation().y << " " << b->transform->GetLocation().z << std::endl;
	std::cout << "Min: " << b->GetWorldMin().x << " " << b->GetWorldMin().y << " " << b->GetWorldMin().z << std::endl;
	std::cout << "Max: " << b->GetWorldMax().x << " " << b->GetWorldMax().y << " " << b->GetWorldMax().z << std::endl;*/
	/*std::cout << "Pos: " << a->transform->GetLocation().x << " " << a->transform->GetLocation().y << " " << a->transform->GetLocation().z << std::endl;
	std::cout << "Min: " << a->GetWorldMin().x << " " << a->GetWorldMin().y << " " << a->GetWorldMin().z << std::endl;
	std::cout << "Max: " << a->GetWorldMax().x << " " << a->GetWorldMax().y << " " << a->GetWorldMax().z << std::endl;*/

	if(out_colliding)
		std::cout << "!" << std::endl;
	else
		std::cout << "?" << std::endl;
}

bool PhysicsSubsystem::GJK(const ConvexCollider* a, const ConvexCollider* b, Vector& out_normal)
{
	out_normal = Vector(0, 1, 0);

	// Arbitrary direction as a starting point
	Vector direction(1, 0, 0);
	//assert((a->GJK_Support(direction) - b->GJK_Support(-direction)) == -(b->GJK_Support(direction) - a->GJK_Support(-direction)));
	//assert();
	// Get the Minkowski Difference on the hull in this direction
	Vector difference = a->GJK_Support(direction) - b->GJK_Support(-direction);
	// Initialize the simplex
	std::deque<Vector> simplex { difference };

	// Get the new direction towards the origin
	direction = -simplex.back();

	// Add up to three more points to the simplex to try to contain the origin
	while(true)
	{
		// Get the next Minkowski Difference on the hull
		Vector diff = a->GJK_Support(direction) - b->GJK_Support(-direction);
		// If we didn't pass the origin, return false
		if(diff.Dot(direction) <= 0)
			return false;

		// Add new point to simplex
		simplex.push_front(diff);

		// Test for the simplex intersecting the origin
		if(UpdateSimplex(simplex, direction))
		{
			//out_normal = direction;
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