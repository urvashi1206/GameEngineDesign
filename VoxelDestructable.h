#pragma once

#include "Component.h"

#include "VoxelGrid.h"

class VoxelDestructable : public Component
{
private:
	VoxelGrid* voxelGrid;

public:
	VoxelDestructable(VoxelGrid* voxelGrid);
	~VoxelDestructable();

	void Fracture();

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;
};