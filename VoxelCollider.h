#pragma once

#include "CompositeCollider.h"

#include "VoxelGrid.h"

class VoxelCollider : public CompositeCollider
{
protected:
	VoxelGrid* voxelGrid;

public:
	VoxelCollider(VoxelGrid* voxelGrid);
	~VoxelCollider();

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;
};