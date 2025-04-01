#include "VoxelCollider.h"

VoxelCollider::VoxelCollider(VoxelGrid* voxelGrid) : CompositeCollider(),
	voxelGrid(voxelGrid)
{
	colliders.push_back(voxelGrid->GetBoxColliderHull());
}
VoxelCollider::~VoxelCollider()
{

}

void VoxelCollider::Initialize()
{
	CompositeCollider::Initialize();
}
void VoxelCollider::Update(float deltaTime)
{
	CompositeCollider::Update(deltaTime);
}