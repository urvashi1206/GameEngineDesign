#include "VoxelGrid.h"

#include <iostream>

VoxelGrid::VoxelGrid(size_t width, size_t height, size_t depth) :
    width(width), height(height), depth(depth), voxelStates(new BoolVoxel[width * height * depth]), voxelColors(new ColorVoxel[width * height * depth]), voxelCount(width * height * depth), 
    collisionOctree(IntVector(0, 0, 0), IntVector((int) width, (int) height, (int) depth)), 
    colorOctree(IntVector(0, 0, 0), IntVector((int) width, (int) height, (int) depth))
{

}
VoxelGrid::VoxelGrid(const VoxelGrid& other) : 
    width(other.width), height(other.height), depth(other.depth), voxelStates(new BoolVoxel[other.voxelCount]), voxelColors(new ColorVoxel[other.voxelCount]), voxelCount(other.voxelCount),
    collisionOctree(collisionOctree), colorOctree(other.colorOctree)
{
    for(size_t i = 0; i < voxelCount; i++)
    {
        voxelStates[i] = other.voxelStates[i];
        voxelColors[i] = other.voxelColors[i];
    }
}
VoxelGrid::~VoxelGrid()
{
    if(voxelStates) delete[] voxelStates;
    if(voxelColors) delete[] voxelColors;

	voxelStates = nullptr;
    voxelColors = nullptr;
}

ColorVoxel& VoxelGrid::operator[](size_t index) const
{
    return voxelColors[index];
}
ColorVoxel& VoxelGrid::operator[](Vector coordinates) const
{
    return voxelColors[(unsigned int) (coordinates.x + coordinates.y * width * depth + coordinates.z * width)];
}

void VoxelGrid::GenerateCollisionOctree()
{
    collisionOctree.Generate(voxelStates, voxelCount);
}
void VoxelGrid::GenerateColorOctree()
{
    colorOctree.Generate(voxelColors, voxelCount);

    /*std::cout << "--- OCTREE VALUES ---" << std::endl;
    for(int y = 0; y < octree.GetDimensions().x; y++)
        for(int z = 0; z < octree.GetDimensions().x; z++)
        {
            for(int x = 0; x < octree.GetDimensions().x; x++)
            {
                std::cout << octree.GetVoxelValue(IntVector(x, y, z)).x << octree.GetVoxelValue(IntVector(x, y, z)).y << " ";
            }
            std::cout << std::endl;
        }*/
}

BoxCollider* VoxelGrid::GetBoxColliderHull() const
{
    if(collisionOctree.GetLeaves().size() == 0)
        return nullptr;

    IntVector minCoords = IntVector(INT_MAX, INT_MAX, INT_MAX);
    IntVector maxCoords = IntVector(INT_MIN, INT_MIN, INT_MIN);

    for(SparseVoxelOctreeNode<bool>* leaf : collisionOctree.GetLeaves())
    {
        IntVector leafMin = leaf->GetBoundsMin();
        IntVector leafMax = leaf->GetBoundsMax();

        if(leafMin.x < minCoords.x)
            minCoords.x = leafMin.x;
        if(leafMin.y < minCoords.y)
            minCoords.y = leafMin.y;
        if(leafMin.z < minCoords.z)
            minCoords.z = leafMin.z;

        if(leafMax.x > maxCoords.x)
            maxCoords.x = leafMax.x;
        if(leafMax.y > maxCoords.y)
            maxCoords.y = leafMax.y;
        if(leafMax.z > maxCoords.z)
            maxCoords.z = leafMax.z;
    }

    Vector min = GridCoordsToNormalizedLocalSpace(minCoords);
    Vector max = GridCoordsToNormalizedLocalSpace(maxCoords);

    Vector center = (min + max) / 2.0f;
    Vector halfSize = (max - min) / 2.0f;
	halfSize.x = abs(halfSize.x);
	halfSize.y = abs(halfSize.y);
	halfSize.z = abs(halfSize.z);

    return new BoxCollider(center, halfSize, true);
}
std::shared_ptr<Mesh> VoxelGrid::GenerateMesh(Material* material) const
{
    static const std::vector<IntVector> CUBE_DIRECTIONS =
    {
        IntVector(-1, 0, 0),    // Left
        IntVector(1, 0, 0),     // Right
        IntVector(0, -1, 0),    // Down
        IntVector(0, 1, 0),     // Up
        IntVector(0, 0, -1),    // Back
        IntVector(0, 0, 1)      // Forward
    };

    //std::vector<std::pair<Vector, Vector>> surfaceRegionsMinMax;
    //octree.GetSurfaceRegions(CUBE_DIRECTIONS, surfaceRegionsMinMax);

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;
    for(SparseVoxelOctreeNode<Vector>* leaf : colorOctree.GetLeaves())
    {
        std::vector<std::pair<Vector, Vector>> surfaceRegionsMinMax;
        leaf->GetSurfaceRegions(CUBE_DIRECTIONS, surfaceRegionsMinMax);

        Vector paletteCoordinates = leaf->GetVoxel(IntVector::Zero())->value;

        for(std::pair<Vector, Vector> regionMinMax : surfaceRegionsMinMax)
        {
            // Transform from grid coordinates to normalized local mesh coordinates
            Vector min = GridCoordsToNormalizedLocalSpace(regionMinMax.first);
            Vector max = GridCoordsToNormalizedLocalSpace(regionMinMax.second);

            // Winding order should be clockwise when viewed from beyond the normal
            
            // Orthogonal vectors
            Vector u, v;
            if(min.x == max.x)
            {
                u = (max - min).ProjectOnto(Vector::Up());
                v = (max - min).ProjectOnto(Vector::Forward());
            }
            else if(min.y == max.y)
            {
                u = (max - min).ProjectOnto(Vector::Forward());
                v = (max - min).ProjectOnto(Vector::Right());
            }
            else if(min.z == max.z)
            {
                u = (max - min).ProjectOnto(Vector::Up());
                v = (max - min).ProjectOnto(Vector::Right());
            }

            Vector normal = u.Cross(v).Normalized();

            vertices.push_back({ DirectX::XMFLOAT3(min.x, min.y, min.z), DirectX::XMFLOAT3(normal.x, normal.y, normal.z), DirectX::XMFLOAT3(), DirectX::XMFLOAT2(paletteCoordinates.x, paletteCoordinates.y) });
            vertices.push_back({ DirectX::XMFLOAT3(min.x + u.x, min.y + u.y, min.z + u.z), DirectX::XMFLOAT3(normal.x, normal.y, normal.z), DirectX::XMFLOAT3(), DirectX::XMFLOAT2(paletteCoordinates.x, paletteCoordinates.y) });
            vertices.push_back({ DirectX::XMFLOAT3(max.x, max.y, max.z), DirectX::XMFLOAT3(), DirectX::XMFLOAT3(normal.x, normal.y, normal.z), DirectX::XMFLOAT2(paletteCoordinates.x, paletteCoordinates.y) });
            vertices.push_back({ DirectX::XMFLOAT3(min.x + v.x, min.y + v.y, min.z + v.z), DirectX::XMFLOAT3(normal.x, normal.y, normal.z), DirectX::XMFLOAT3(), DirectX::XMFLOAT2(paletteCoordinates.x, paletteCoordinates.y) });

            indices.push_back((UINT) vertices.size() - 4);
            indices.push_back((UINT) vertices.size() - 3);
            indices.push_back((UINT) vertices.size() - 2);
            indices.push_back((UINT) vertices.size() - 4);
            indices.push_back((UINT) vertices.size() - 2);
            indices.push_back((UINT) vertices.size() - 1);
        }
    }

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>("Mesh", vertices, indices);
    return mesh;
}

std::unordered_map<IntVector, VoxelGrid*> VoxelGrid::PartitionGrid_Voronoi(size_t fractureCount)
{
    std::vector<IntVector> points;
    for(size_t i = 0; i < fractureCount; i++)
        points.push_back(IntVector::Rand(colorOctree.GetBoundsMin(), colorOctree.GetBoundsMax()));

    std::unordered_map<IntVector, VoxelGrid*> pointGroups;
    for(IntVector point : points)
        pointGroups.emplace(point, new VoxelGrid(width, height, depth));

    for(size_t y = 0; y < height; y++)
        for(size_t z = 0; z < depth; z++)
            for(size_t x = 0; x < width; x++)
            {
                bool voxelState = collisionOctree.GetVoxelValue(IntVector(x, y, z));
                Vector voxelColor = colorOctree.GetVoxelValue(IntVector(x, y, z));
                if(!voxelState)
                    continue;

                // Find closest point
                IntVector closestPoint;
                float minDistance = FLT_MAX;
                for(IntVector point : points)
                {
                    float distance = Vector(x, y, z).Distance(point);
                    if(distance < minDistance)
                    {
                        closestPoint = point;
                        minDistance = distance;
                    }
                }

                (*pointGroups[closestPoint]).voxelStates[x + y * width * depth + z * width] = BoolVoxel(true);
                (*pointGroups[closestPoint])[x + y * width * depth + z * width] = ColorVoxel(voxelColor);
            }

    return pointGroups;
}

Vector VoxelGrid::GridCoordsToNormalizedLocalSpace(const IntVector& gridCoords) const
{
    float hullSize = max(width, height, depth);

    return (Vector(gridCoords) - Vector(hullSize, hullSize, hullSize) / 2.0f) / Vector(hullSize, -1.0f * hullSize, -1.0f * hullSize, 1);
}