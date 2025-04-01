#pragma once

#include "Renderer.h"

#include "VoxelGrid.h"

class VoxelRenderer : public Renderer
{
private:
	VoxelGrid* voxelGrid;

public:
	VoxelRenderer(VoxelGrid* voxelGrid, std::shared_ptr<Material> material);
	~VoxelRenderer();
};