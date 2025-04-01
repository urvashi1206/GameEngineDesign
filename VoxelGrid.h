#pragma once

#include "Voxel.h"
#include "Vector.h"
#include "Octree.h"
#include "Mesh.h"
#include "Material.h"
#include "Color.h"
#include "BoxCollider.h"

// Voxel encoding color using the UV coordinates of a palette texture
using ColorVoxel = Voxel<Vector>;
using BoolVoxel = Voxel<bool>;

class VoxelGrid
{
private:
	size_t width;
	size_t height;
	size_t depth;
	BoolVoxel* voxelStates;
	ColorVoxel* voxelColors;
	size_t voxelCount;
	std::unordered_map<Color, Vector> palette;

	SparseVoxelOctree<bool> collisionOctree;
	SparseVoxelOctree<Vector> colorOctree;

private:
	VoxelGrid(size_t width, size_t height, size_t depth);
	VoxelGrid(const VoxelGrid& other);
	~VoxelGrid();

	friend class ResourceManager;

public:
	ColorVoxel& operator[](size_t index) const;
	ColorVoxel& operator[](Vector coordinates) const;

	void GenerateCollisionOctree();
	void GenerateColorOctree();

	BoxCollider* GetBoxColliderHull() const;
	std::shared_ptr<Mesh> GenerateMesh(Material* material) const;

	std::unordered_map<IntVector, VoxelGrid*> PartitionGrid_Voronoi(size_t fractureCount);

	Vector GridCoordsToNormalizedLocalSpace(const IntVector& gridCoords) const;
};