#pragma once

#include "Voxel.h"

class VoxelGrid
{
private:
	Voxel* voxels;

public:
	VoxelGrid(unsigned int width, unsigned int height, unsigned int depth);
	~VoxelGrid();

	void GenerateOctree();

	void PartitionGrid_Voronoi(unsigned int fractureCount);
};