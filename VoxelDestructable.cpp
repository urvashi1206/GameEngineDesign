#include "VoxelDestructable.h"

#include "VoxelRenderer.h"
#include "VoxelCollider.h"
#include "Rigidbody.h"
#include "PhysicsSubsystem.h"
#include "ResourceManager.h"
#include "EntityManager.h"

VoxelDestructable::VoxelDestructable(VoxelGrid* voxelGrid) : 
	voxelGrid(voxelGrid)
{

}
VoxelDestructable::~VoxelDestructable()
{

}

void VoxelDestructable::Fracture()
{
	std::unordered_map<IntVector, VoxelGrid*> fractureGroups = voxelGrid->PartitionGrid_Voronoi(4);
	for(auto& fractureGroup : fractureGroups)
	{
		fractureGroup.second->GenerateCollisionOctree();

		Entity* fractureEntity = new Entity();
		fractureEntity->AddComponent(new Transform(Vector::Zero(), Vector::Zero(), Vector::One()));
		//fractureEntity->AddComponent(new VoxelRenderer(fractureGroup.second, ResourceManager::GetMaterialByID(8)));
		fractureEntity->AddComponent(fractureGroup.second->GetBoxColliderHull());
		fractureEntity->AddComponent(new Rigidbody(Vector(0, -9.81f, 0)));
		EntityManager::AddEntity(fractureEntity);

		PhysicsSubsystem::AddRigidbody(fractureEntity->GetComponent<Rigidbody>());
	}
}

void VoxelDestructable::Initialize()
{

}
void VoxelDestructable::Update(float deltaTime)
{

}