#include "VoxelRenderer.h"

VoxelRenderer::VoxelRenderer(VoxelGrid* voxelGrid, std::shared_ptr<Material> material) : Renderer(nullptr, material),
	voxelGrid(voxelGrid)
{
	voxelGrid->GenerateColorOctree();
	mesh = voxelGrid->GenerateMesh(material.get());
}
VoxelRenderer::~VoxelRenderer()
{

}