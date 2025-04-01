#pragma once

#include <vector>
#include <unordered_map>

#include "VoxelGrid.h"
#include "IntVector.h"

const std::vector<IntVector> OCTANTS =
{
	IntVector(0, 0, 0),
	IntVector(1, 0, 0),
	IntVector(0, 1, 0),
	IntVector(1, 1, 0),
	IntVector(0, 0, 1),
	IntVector(1, 0, 1),
	IntVector(0, 1, 1),
	IntVector(1, 1, 1)
};

#pragma region Octree
template<class T>
struct OctreeNode
{
private:
	Vector min, max;
	std::vector<OctreeNode<T>> children;
};

template<class T>
struct Octree
{
private:
	OctreeNode<T> root;

public:
	Octree();
	~Octree();
};

template<class T>
inline Octree<T>::Octree()
{

}
template<class T>
inline Octree<T>::~Octree()
{

}
#pragma endregion

#pragma region SparseVoxelOctree

#pragma region Headers
template<class T>
struct SparseVoxelOctree;
template<class T>
struct SparseVoxelOctreeNode;

template<class T>
struct SparseVoxelOctree
{
private:
	IntVector boundsMin, boundsMax;
	SparseVoxelOctreeNode<T> root;

	std::vector<SparseVoxelOctreeNode<T>*> leaves;

public:
	SparseVoxelOctree(IntVector boundsMin = IntVector(), IntVector boundsMax = IntVector());
	SparseVoxelOctree(const SparseVoxelOctree<T>& other);
	~SparseVoxelOctree();

	void Generate(Voxel<T>* data, size_t size);

	T GetVoxelValue(IntVector coordinates) const;
	const SparseVoxelOctreeNode<T>* FindNode(IntVector coordinates, size_t targetDepth) const;

	IntVector GetBoundsMin() const { return boundsMin; };
	IntVector GetBoundsMax() const { return boundsMax; };
	IntVector GetDimensions() const { return boundsMax - boundsMin; };
	std::vector<SparseVoxelOctreeNode<T>*> GetLeaves() const { return leaves; };

	void GetSurfaceRegions(std::vector<IntVector> directions, std::vector<std::pair<Vector, Vector>>& inout_surfaceRegionsMinMax) const;

	// Subdivide and move operator modify octree leaves
	friend bool SparseVoxelOctreeNode<T>::Subdivide();
	friend SparseVoxelOctreeNode<T>& SparseVoxelOctreeNode<T>::operator=(SparseVoxelOctreeNode<T>&& other) noexcept;
};

template<class T>
struct SparseVoxelOctreeNode
{
private:
	SparseVoxelOctree<T>* octree;
	SparseVoxelOctreeNode<T>* parent;
	IntVector localOctant;
	size_t depth;

	IntVector boundsMin, boundsMax;
	Voxel<T>* data;
	size_t size;

	std::unordered_map<IntVector, SparseVoxelOctreeNode<T>> children;

public:
	SparseVoxelOctreeNode();
	SparseVoxelOctreeNode(const SparseVoxelOctreeNode<T>& other);
	SparseVoxelOctreeNode(SparseVoxelOctreeNode<T>&& other);
	SparseVoxelOctreeNode(
		SparseVoxelOctree<T>* octree, SparseVoxelOctreeNode<T>* parent, IntVector localOctant, 
		Voxel<T>* data, size_t size, IntVector boundsMin, IntVector boundsMax);
	~SparseVoxelOctreeNode();

	bool Subdivide();

	SparseVoxelOctreeNode<T>& operator=(const SparseVoxelOctreeNode<T>& other);
	SparseVoxelOctreeNode<T>& operator=(SparseVoxelOctreeNode<T>&& other) noexcept;
	
	bool operator==(const SparseVoxelOctreeNode<T>& other) const;
	
	Voxel<T>* GetVoxel(IntVector localCoordinates) const;
	T GetVoxelValue(IntVector coordinates) const;
	const SparseVoxelOctreeNode<T>* FindNode(IntVector coordinates, size_t targetDepth) const;

	void GetEmptyFaceRegions(const IntVector& faceNormal, std::vector<std::pair<IntVector, IntVector>>& emptyRegionsMinMax) const;
	
	void GetSurfaceRegions(std::vector<IntVector> directions, std::vector<std::pair<Vector, Vector>>& inout_surfaceRegionsMinMax) const;

	IntVector GetBoundsMin() const { return boundsMin; };
	IntVector GetBoundsMax() const { return boundsMax; };
	bool IsLeaf() const { return children.size() == 0; };

	void SetParent(SparseVoxelOctreeNode<T>* parent) { this->parent = parent; };

	static std::vector<IntVector> GetValidOctants(IntVector nodeSize);
	static void GetFaceRegion(IntVector boundsMin, IntVector boundsMax, IntVector direction, IntVector& out_regionMin, IntVector& out_regionMax);
};
#pragma endregion

#pragma region Definitions
template<class T>
inline SparseVoxelOctree<T>::SparseVoxelOctree(IntVector boundsMin, IntVector boundsMax) : 
	boundsMin(boundsMin), boundsMax(boundsMax), root(), leaves()
{

}
template<class T>
inline SparseVoxelOctree<T>::SparseVoxelOctree(const SparseVoxelOctree<T>& other) : 
	boundsMin(other.boundsMin), boundsMax(other.boundsMax), root(), leaves()
{

}
template<class T>
inline SparseVoxelOctree<T>::~SparseVoxelOctree()
{

}
template<class T>
inline void SparseVoxelOctree<T>::Generate(Voxel<T>* data, size_t size)
{
	root = SparseVoxelOctreeNode<T>(this, nullptr, IntVector::Zero(), data, size, boundsMin, boundsMax);
	root.Subdivide();
}
template<class T>
inline T SparseVoxelOctree<T>::GetVoxelValue(IntVector coordinates) const
{
	// If the voxel is outside the octree, its value is 0
	if(coordinates.x < boundsMin.x || coordinates.x >= boundsMax.x 
		|| coordinates.y < boundsMin.y || coordinates.y >= boundsMax.y 
		|| coordinates.z < boundsMin.z || coordinates.z >= boundsMax.z)
		return 0;

	// Recursively traverse the octree
	return root.GetVoxelValue(coordinates);
}
template<class T>
inline const SparseVoxelOctreeNode<T>* SparseVoxelOctree<T>::FindNode(IntVector coordinates, size_t targetDepth) const
{
	// If the voxel is outside the octree, return nullptr
	if(coordinates.x < boundsMin.x || coordinates.x >= boundsMax.x
		|| coordinates.y < boundsMin.y || coordinates.y >= boundsMax.y
		|| coordinates.z < boundsMin.z || coordinates.z >= boundsMax.z)
		return nullptr;

	// Recursively traverse the octree
	return root.FindNode(coordinates, targetDepth);
}
template<class T>
inline void SparseVoxelOctree<T>::GetSurfaceRegions(std::vector<IntVector> directions, std::vector<std::pair<Vector, Vector>>& inout_surfaceRegionsMinMax) const
{
	root.GetSurfaceRegions(directions, inout_surfaceRegionsMinMax);
}
template<class T>
inline SparseVoxelOctreeNode<T>::SparseVoxelOctreeNode() : 
	octree(nullptr), parent(nullptr), localOctant(), depth(0), data(nullptr), size(0), boundsMin(), boundsMax(), children()
{

}
template<class T>
inline SparseVoxelOctreeNode<T>::SparseVoxelOctreeNode(const SparseVoxelOctreeNode<T>& other) : 
	octree(other.octree), parent(other.parent), localOctant(other.localOctant), depth(other.depth), 
	data(other.data), size(other.size), boundsMin(other.boundsMin), boundsMax(other.boundsMax), children()
{
	for(auto& child : other.children)
		children[child.first] = child.second;

	for(auto& child : children)
		child.second.parent = this;
}
template<class T>
inline SparseVoxelOctreeNode<T>::SparseVoxelOctreeNode(SparseVoxelOctreeNode<T>&& other) : 
	octree(std::move(other.octree)), parent(std::move(other.parent)), localOctant(std::move(other.localOctant)), depth(std::move(other.depth)), 
	data(std::move(other.data)), size(std::move(other.size)), boundsMin(std::move(other.boundsMin)), boundsMax(std::move(other.boundsMax)), children()
{
	//for(auto& child : other.children)
		//children[child.first] = child.second;
}
template<class T>
inline SparseVoxelOctreeNode<T>::SparseVoxelOctreeNode(
	SparseVoxelOctree<T>* octree, SparseVoxelOctreeNode<T>* parent, IntVector localOctant,
	Voxel<T>* data, size_t size, IntVector boundsMin, IntVector boundsMax) :
	octree(octree), parent(parent), localOctant(localOctant), depth(0), 
	data(data), size(size), boundsMin(boundsMin), boundsMax(boundsMax), 
	children()
{
	depth = parent ? parent->depth + 1 : 0;
}
template<class T>
inline SparseVoxelOctreeNode<T>::~SparseVoxelOctreeNode()
{
	
}
template<class T>
inline bool SparseVoxelOctreeNode<T>::Subdivide()
{
	// Check status of this node
	// 1. If empty, return false - this node will be removed from octree
	// 2. If solid, return true - this node will be kept and will not subdivide further
	// 3. If mixed, subdivide further
	T octreeNodeValue = GetVoxel(IntVector::Zero())->value;
	bool isMixed = false;
	for(int y = 0; y < boundsMax.y - boundsMin.y; y++)
	{
		for(int z = 0; z < boundsMax.z - boundsMin.z; z++)
		{
			for(int x = 0; x < boundsMax.x - boundsMin.x; x++)
			{
				if(!(GetVoxel(IntVector(x, y, z))->value == octreeNodeValue))
				{
					// No further iterations are needed if we know this node is mixed
					isMixed = true;
					break;
				}
			}
			// No further iterations are needed if we know this node is mixed
			if(isMixed)
				break;
		}
		// No further iterations are needed if we know this node is mixed
		if(isMixed)
			break;
	}
	// If the node isn't mixed, it's either solid or empty
	if(!isMixed)
	{
		// Default T value is used to designate an empty voxel
		if(octreeNodeValue == T())
			return false;
		else
		{
			octree->leaves.push_back(this);
			return true;
		}
	}

	/* Subdivide */

	std::vector<IntVector> validOctants = GetValidOctants(boundsMax - boundsMin);

	for(IntVector octant : validOctants)
	{
		Vector octantSize = (boundsMax - boundsMin) / 2.0f;
		IntVector octantMin = ((Vector) boundsMin + octant * octantSize).Ceil();
		IntVector octantMax = ((Vector) boundsMax - (Vector::One() - octant) * octantSize).Ceil();

		SparseVoxelOctreeNode<T> child(octree, this, octant, data, (size_t) (octantSize.x * octantSize.y * octantSize.z), octantMin, octantMax);
		if(child.Subdivide())
			children[octant] = std::move(child);
	}

	// This node is not empty
	return true;
}
template<class T>
inline Voxel<T>* SparseVoxelOctreeNode<T>::GetVoxel(IntVector localCoordinates) const
{
	return &data[(boundsMin.x + localCoordinates.x) + (boundsMin.y + localCoordinates.y) * octree->GetDimensions().x * octree->GetDimensions().z + (boundsMin.z + localCoordinates.z) * octree->GetDimensions().x];
}
template<class T>
inline T SparseVoxelOctreeNode<T>::GetVoxelValue(IntVector coordinates) const
{
	// If this node is a leaf, return its value
	if(IsLeaf())
		return GetVoxel(IntVector::Zero())->value;

	// Determine next child octant
	IntVector nodeCenter = ((Vector) boundsMin + (boundsMax - boundsMin) / 2.0f).Ceil();
	IntVector octantDir((coordinates.x < nodeCenter.x) ? 0 : 1, (coordinates.y < nodeCenter.y) ? 0 : 1, (coordinates.z < nodeCenter.z) ? 0 : 1);

	// If the specified octant is empty, return default value
	if(children.find(octantDir) == children.end())
		return T();

	return children.at(octantDir).GetVoxelValue(coordinates);
}
template<class T>
inline const SparseVoxelOctreeNode<T>* SparseVoxelOctreeNode<T>::FindNode(IntVector coordinates, size_t targetDepth) const
{
	// If depth limit is reached, stop here
	if(depth >= targetDepth)
		return this;

	// If this node is a leaf, we can't go any further
	if(IsLeaf())
		return nullptr;

	// Determine next child octant
	IntVector nodeCenter = ((Vector) boundsMin + (boundsMax - boundsMin) / 2.0f).Ceil();
	IntVector octantDir((coordinates.x < nodeCenter.x) ? 0 : 1, (coordinates.y < nodeCenter.y) ? 0 : 1, (coordinates.z < nodeCenter.z) ? 0 : 1);

	// If the specified octant is empty, return nullptr
	if(children.find(octantDir) == children.end())
		return nullptr;

	return children.at(octantDir).FindNode(coordinates, targetDepth);
}
template<class T>
inline void SparseVoxelOctreeNode<T>::GetEmptyFaceRegions(const IntVector& faceNormal, std::vector<std::pair<IntVector, IntVector>>& emptyRegionsMinMax) const
{
	// If this is a leaf node, it has no empty face regions
	if(IsLeaf())
		return;

	std::vector<IntVector> validOctants = GetValidOctants(boundsMax - boundsMin);

	// Gather the octants that are touching the relevant face
	std::vector<IntVector> relevantOctants;
	for(IntVector octant : validOctants)
		// If moving from the octant in the face direction moves outside of the node, the octant is touching the face
		if(std::find(validOctants.begin(), validOctants.end(), octant + faceNormal) == validOctants.end())
			relevantOctants.push_back(octant);

	// Check each octant for emptiness
	for(IntVector octant : relevantOctants)
	{
		// If the octant node is empty (doesn't exist), add it as an empty region
		if(children.find(octant) == children.end())
		{
			// Get the bounds of the missing node and add it to empty regions
			Vector octantSize = (boundsMax - boundsMin) / 2.0f;
			IntVector octantMin = ((Vector) boundsMin + octant * octantSize).Ceil();
			IntVector octantMax = ((Vector) boundsMax - (Vector::One() - octant) * octantSize).Ceil();

			IntVector faceMin, faceMax;
			GetFaceRegion(octantMin, octantMax, faceNormal, faceMin, faceMax);
			emptyRegionsMinMax.emplace_back(faceMin, faceMax);
		}
		// Otherwise continue recursion
		else
			children.at(octant).GetEmptyFaceRegions(faceNormal, emptyRegionsMinMax);
	}
}
template<class T>
inline void SparseVoxelOctreeNode<T>::GetSurfaceRegions(std::vector<IntVector> directions, std::vector<std::pair<Vector, Vector>>& inout_surfaceRegionsMinMax) const
{
	// Traverse down to leaves
	if(!IsLeaf())
	{
		for(auto& child : children)
			child.second.GetSurfaceRegions(directions, inout_surfaceRegionsMinMax);

		return;
	}
	// If the root node is the only node, return with all faces
	if(!parent)
	{
		for(IntVector dir : directions)
		{
			IntVector faceMin, faceMax;
			GetFaceRegion(boundsMin, boundsMax, dir, faceMin, faceMax);
			inout_surfaceRegionsMinMax.push_back(std::pair<IntVector, IntVector>(faceMin, faceMax));
		}
		return;
	}

	/* Leaf nodes */

	// Find "surface" face regions in all directions
	for(IntVector dir : directions)
	{
		/* Locate the relevant adjacent octree node on the same level as this node */

		std::vector<std::pair<IntVector, IntVector>> solidRegionsMinMax;
		std::vector<std::pair<IntVector, IntVector>> emptyRegionsMinMax;
			
		IntVector octantSize = boundsMax - boundsMin;

		IntVector adjacentBoundsMin = boundsMin + dir * octantSize;
		IntVector adjacentBoundsMax = boundsMax + dir * octantSize;

		// Get the adjacent octree node at this depth
		const SparseVoxelOctreeNode<T>* adjacent = octree->FindNode(adjacentBoundsMin, depth);
		if(adjacent)
			adjacent->GetEmptyFaceRegions(-dir, emptyRegionsMinMax);
		// Otherwise there is no adjacent node; add the full face
		else
		{
			IntVector faceMin, faceMax;
			GetFaceRegion(boundsMin, boundsMax, dir, faceMin, faceMax);
			inout_surfaceRegionsMinMax.emplace_back(faceMin, faceMax);
		}

		// Shift the min and max coordinates into this node's region
		// Also convert surface coordinates to exact points on the node's boundary
		for(std::pair<IntVector, IntVector>& regionMinMax : emptyRegionsMinMax)
		{
			Vector diagonal = Vector(regionMinMax.first - regionMinMax.second).Normalized();
			Vector normal = Vector(dir).Normalized();

			Vector reflectionAxis = diagonal + normal.Cross(diagonal);

			Vector midPoint = (regionMinMax.first + regionMinMax.second) / 2.0f;
			Vector min = Vector(regionMinMax.first).Reflect(reflectionAxis, midPoint);
			Vector max = Vector(regionMinMax.second).Reflect(reflectionAxis, midPoint);
			inout_surfaceRegionsMinMax.emplace_back(min, max);
		}
	}
}
template<class T>
inline SparseVoxelOctreeNode<T>& SparseVoxelOctreeNode<T>::operator=(const SparseVoxelOctreeNode<T>& other)
{
	octree = other.octree;
	parent = other.parent;
	localOctant = other.localOctant;
	depth = other.depth;
	data = other.data;
	size = other.size;
	boundsMin = other.boundsMin;
	boundsMax = other.boundsMax;

	for(auto& child : other.children)
		children[child.first] = child.second;

	for(auto& child : children)
		child.second.parent = this;

	return *this;
}
template<class T>
inline SparseVoxelOctreeNode<T>& SparseVoxelOctreeNode<T>::operator=(SparseVoxelOctreeNode<T>&& other) noexcept
{
	if(this != &other)
    {
        octree = std::move(other.octree);
        parent = std::move(other.parent);
        localOctant = std::move(other.localOctant);
        depth = std::move(other.depth);
        data = std::move(other.data);
        size = std::move(other.size);
        boundsMin = std::move(other.boundsMin);
        boundsMax = std::move(other.boundsMax);

        for(auto& child : other.children)
            children[child.first] = std::move(child.second);

        for(auto& child : children)
            child.second.parent = this;

        if(IsLeaf())
        {
            // Find the leaf index
            auto& leaves = octree->leaves;
            auto it = std::find(leaves.begin(), leaves.end(), &other);
            if(it != leaves.end())
            {
                *it = this; // Replace the pointer to "other" with a pointer to "this"
            }
        }

        // Clear the other node's children to avoid dangling pointers
        other.children.clear();
    }

	return *this;
}
template<class T>
inline bool SparseVoxelOctreeNode<T>::operator==(const SparseVoxelOctreeNode<T>& other) const
{
	return this->octree == other.octree && this->boundsMin == other.boundsMin && this->boundsMax == other.boundsMax;
}
template<class T>
inline std::vector<IntVector> SparseVoxelOctreeNode<T>::GetValidOctants(IntVector nodeSize)
{
	std::vector<IntVector> validOctants = OCTANTS;
	if(nodeSize.x == 1)
	{
		static const std::vector<IntVector> OCTANTS_X =
		{
			IntVector(1, 0, 0),
			IntVector(1, 0, 1),
			IntVector(1, 1, 0),
			IntVector(1, 1, 1)
		};

		// Remove all octants with non-zero x
		auto it = validOctants.end();
		for(IntVector octant : OCTANTS_X)
		{
			it = std::find(validOctants.begin(), validOctants.end(), octant);
			if(it != validOctants.end())
				validOctants.erase(it);
		}
	}
	if(nodeSize.y == 1)
	{
		static const std::vector<IntVector> OCTANTS_Y =
		{
			IntVector(0, 1, 0),
			IntVector(0, 1, 1),
			IntVector(1, 1, 0),
			IntVector(1, 1, 1)
		};

		// Remove all octants with non-zero y
		auto it = validOctants.end();
		for(IntVector octant : OCTANTS_Y)
		{
			it = std::find(validOctants.begin(), validOctants.end(), octant);
			if(it != validOctants.end())
				validOctants.erase(it);
		}
	}
	if(nodeSize.z == 1)
	{
		static const std::vector<IntVector> OCTANTS_Z =
		{
			IntVector(0, 0, 1),
			IntVector(0, 1, 1),
			IntVector(1, 0, 1),
			IntVector(1, 1, 1)
		};

		// Remove all octants with non-zero z
		auto it = validOctants.end();
		for(IntVector octant : OCTANTS_Z)
		{
			it = std::find(validOctants.begin(), validOctants.end(), octant);
			if(it != validOctants.end())
				validOctants.erase(it);
		}
	}

	return validOctants;
}
template<class T>
inline void SparseVoxelOctreeNode<T>::GetFaceRegion(IntVector boundsMin, IntVector boundsMax, IntVector direction, IntVector& out_regionMin, IntVector& out_regionMax)
{
	// Right/left
	if(direction.x != 0)
	{
		// Right face region
		out_regionMin = IntVector(boundsMax.x, boundsMin.y, boundsMin.z);
		out_regionMax = IntVector(boundsMax.x, boundsMax.y, boundsMax.z);

		// If negative, reflect X and Z
		if(direction.x < 0)
		{
			out_regionMin = IntVector(boundsMin.x, boundsMin.y, boundsMax.z);
			out_regionMax = IntVector(boundsMin.x, boundsMax.y, boundsMin.z);
		}
	}
	// Up/down
	else if(direction.y != 0)
	{
		// Up face region
		out_regionMin = IntVector(boundsMin.x, boundsMax.y, boundsMin.z);
		out_regionMax = IntVector(boundsMax.x, boundsMax.y, boundsMax.z);

		// If negative, reflect Y and Z
		if(direction.y < 0)
		{
			out_regionMin = IntVector(boundsMin.x, boundsMin.y, boundsMax.z);
			out_regionMax = IntVector(boundsMax.x, boundsMin.y, boundsMin.z);
		}
	}
	// Front/back
	else if(direction.z != 0)
	{
		// Front face region
		out_regionMin = IntVector(boundsMax.x, boundsMin.y, boundsMax.z);
		out_regionMax = IntVector(boundsMin.x, boundsMax.y, boundsMax.z);

		// If negative, reflect X and Z
		if(direction.z < 0)
		{
			out_regionMin = IntVector(boundsMin.x, boundsMin.y, boundsMin.z);
			out_regionMax = IntVector(boundsMax.x, boundsMax.y, boundsMin.z);
		}
	}
}
#pragma endregion

#pragma endregion