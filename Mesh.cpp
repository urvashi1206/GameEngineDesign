#include "rapidobj/rapidobj.hpp"

#include "Mesh.h"
#include "Graphics.h"
#include "PathHelpers.h"

#include <iostream>
#include <fstream>

#include <DirectXMath.h>


using namespace DirectX;

Mesh::Mesh(std::string name, UINT vertexCount, Vertex vertices[], UINT indexCount, UINT indices[]) :
	name(name), vertexCount(vertexCount), indexCount(indexCount), vertices(vertices), indices(indices)
{
	CalculateTangents(vertices, vertexCount, indices, indexCount);
	CreateBuffers(vertices, vertexCount, indices, indexCount);
}
Mesh::Mesh(const wchar_t* filePath)
{
	//rapidobj::Result obj1 = rapidobj::ParseFile(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str());
	//rapidobj::Result obj1 = rapidobj::ParseFile(FixPath(L"../../Assets/Models/KhaimBook.obj").c_str());
	rapidobj::Result obj1 = rapidobj::ParseFile(filePath);

	std::cout << "SHAPES: " << obj1.shapes.size() << std::endl;
	/*std::cout << "POINTS: " << obj1.shapes[0].points.indices.size() << std::endl;
	std::cout << "LINES: " << obj1.shapes[0].lines.indices.size() << std::endl;
	std::cout << "MESH: " << obj1.shapes[0].mesh.indices.size() << std::endl;*/
	std::cout << "MATERIALS: " << obj1.materials.size() << std::endl;
	//std::cout << "L" << obj1.materials[0].diffuse_texname << std::endl;

	std::vector<XMFLOAT3> positions1;
	std::vector<XMFLOAT3> normals1;
	std::vector<XMFLOAT2> uvs1;
	std::vector<Vertex> vertices1;
	std::vector<UINT> indices1;
	std::unordered_map<Vertex, UINT> vertexMap;

	// For each shape
	for(rapidobj::Shape& shape : obj1.shapes)
	{
		uint32_t firstFaceIndex = 0;
		
		// For each face
		for(int f = 0; f < shape.mesh.num_face_vertices.size(); f++)
		{
			// For each vertex
			for(int j = shape.mesh.num_face_vertices[f] - 1; j >= 0; j--)
			//for(int j = 0; j < shape.mesh.num_face_vertices[f]; j++)
			{
				rapidobj::Index index = shape.mesh.indices[firstFaceIndex + j];

				XMFLOAT3 position = { obj1.attributes.positions[index.position_index * 3], obj1.attributes.positions[index.position_index * 3 + 1], obj1.attributes.positions[index.position_index * 3 + 2] };
				XMFLOAT3 normal = { obj1.attributes.normals[index.normal_index * 3], obj1.attributes.normals[index.normal_index * 3 + 1], obj1.attributes.normals[index.normal_index * 3 + 2] };
				XMFLOAT2 uv = { obj1.attributes.texcoords[index.texcoord_index * 2], obj1.attributes.texcoords[index.texcoord_index * 2 + 1] };

				/*std::cout << "l" << std::endl;
				std::cout << position.x << " " << position.y << " " << position.z << std::endl;
				std::cout << normal.x << " " << normal.y << " " << normal.z << std::endl;
				std::cout << uv.x << " " << uv.y << std::endl;*/

				Vertex vertex = {};
				vertex.position = position;
				vertex.normal = normal;
				vertex.uv = uv;

				vertex.position.z *= -1.0f;
				vertex.normal.z *= -1.0f;

				if(vertexMap.count(vertex) == 0)
				{
					vertexMap[vertex] = static_cast<UINT>(vertices1.size());
					vertices1.push_back(vertex);
				}

				indices1.push_back(vertexMap[vertex]);
			}

			firstFaceIndex += shape.mesh.num_face_vertices[f];
		}
	}

	//rapidobj::Mesh& m = obj1.shapes[0].mesh;
	//std::vector<int*> objIndices;
	//for(int i = 0; i < m.indices.size(); i++)
	//{
	//	rapidobj::Index index = m.indices[i];

	//	//std::cout << obj1.attributes.positions[i] << std::endl;
	//	for()

	//	XMFLOAT3 position = { obj1.attributes.positions[index.position_index * 3], obj1.attributes.positions[index.position_index * 3 + 1], obj1.attributes.positions[index.position_index * 3 + 2] };
	//	XMFLOAT3 normal = { obj1.attributes.normals[index.normal_index * 3], obj1.attributes.normals[index.normal_index * 3 + 1], obj1.attributes.normals[index.normal_index * 3 + 2] };
	//	XMFLOAT2 uv = { obj1.attributes.texcoords[index.texcoord_index * 2], obj1.attributes.texcoords[index.texcoord_index * 2 + 1] };

	//	positions1.push_back(position);
	//	normals1.push_back(normal);
	//	uvs1.push_back(uv);

	//	Vertex vertex = {};
	//	vertex.position = position;
	//	vertex.normal = normal;
	//	vertex.uv = uv;

	//	//auto it = std::find(vertices1.begin(), vertices1.end(), vertex);
	//	//if(it == vertices1.end())
	//		//vertices1.push_back(vertex);

	//	if(vertexMap.count(vertex) == 0)
	//	{
	//		vertexMap[vertex] = static_cast<UINT>(vertices1.size());
	//		vertices1.push_back(vertex);
	//	}

	//	//vertices1.push_back(vertex);

	//	//int k = std::distance(vertices1.begin(), it);

	//	//auto it = std::find(objIndices.begin(), objIndices.end(), { index.position_index, index.normal_index, index.texcoord_index });
	//	//int k = it - objIndices.begin();

	//	//std::cout << "JKJ: " << k << std::endl;

	//	//std::cout << "alwdkalwdk " << vertexMap[vertex] << std::endl;

	//	indices1.push_back(vertexMap[vertex]);
	//}

	/*for(int i = 0; i < obj1.attributes.positions.size(); i += 3)
	{
		std::cout << obj1.attributes.positions[i] << std::endl;

		XMFLOAT3 position = { obj1.attributes.positions[i], obj1.attributes.positions[i + 1], obj1.attributes.positions[i + 2] };
		XMFLOAT3 normal = { obj1.attributes.normals[i], obj1.attributes.normals[i + 1], obj1.attributes.normals[i + 2] };
		XMFLOAT2 uv = { obj1.attributes.texcoords[i], obj1.attributes.texcoords[i + 1] };

		positions1.push_back(position);
		normals1.push_back(normal);
		uvs1.push_back(uv);

		Vertex vertex = {};
		vertex.position = position;
		vertex.normal = normal;
		vertex.uv = uv;

		vertices1.push_back(vertex);
		indices1.push_back(obj1.shapes[0].mesh.indices[0].);
	}*/

	this->vertexCount = vertices1.size();
	this->indexCount = indices1.size();
	this->vertices = &vertices1[0];
	this->indices = &indices1[0];

	//CalculateTangents(&vertices1[0], vertexCount, &indices1[0], indexCount);
	CreateBuffers(&vertices1[0], vertexCount, &indices1[0], indexCount);

	return;

	// Author: Chris Cascioli
	// Purpose: Basic .OBJ 3D model loading, supporting positions, uvs and normals

	// File input object
	std::ifstream obj(filePath);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;	// Positions from the file
	std::vector<XMFLOAT3> normals;		// Normals from the file
	std::vector<XMFLOAT2> uvs;		// UVs from the file
	std::vector<Vertex> verts;		// Verts we're assembling
	std::vector<UINT> indices;		// Indices of these verts
	int vertCounter = 0;			// Count of vertices
	int indexCounter = 0;			// Count of indices
	char chars[100];			// String for line reading

	// Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			// NOTE: This assumes the given obj file contains
			//  vertex positions, uv coordinates AND normals.
			unsigned int i[12];
			int numbersRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// If we only got the first number, chances are the OBJ
			// file has no UV coordinates.  This isn't great, but we
			// still want to load the model without crashing, so we
			// need to re-read a different pattern (in which we assume
			// there are no UVs denoted for any of the vertices)
			if (numbersRead == 1)
			{
				// Re-read with a different pattern
				numbersRead = sscanf_s(
					chars,
					"f %d//%d %d//%d %d//%d %d//%d",
					&i[0], &i[2],
					&i[3], &i[5],
					&i[6], &i[8],
					&i[9], &i[11]);

				// The following indices are where the UVs should 
				// have been, so give them a valid value
				i[1] = 1;
				i[4] = 1;
				i[7] = 1;
				i[10] = 1;

				// If we have no UVs, create a single UV coordinate
				// that will be used for all vertices
				if (uvs.size() == 0)
					uvs.push_back(XMFLOAT2(0, 0));
			}

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.position = positions[i[0] - 1];
			v1.uv = uvs[i[1] - 1];
			v1.normal = normals[i[2] - 1];

			Vertex v2;
			v2.position = positions[i[3] - 1];
			v2.uv = uvs[i[4] - 1];
			v2.normal = normals[i[5] - 1];

			Vertex v3;
			v3.position = positions[i[6] - 1];
			v3.uv = uvs[i[7] - 1];
			v3.normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.uv.y = 1.0f - v1.uv.y;
			v2.uv.y = 1.0f - v2.uv.y;
			v3.uv.y = 1.0f - v3.uv.y;

			// Flip Z (LH vs. RH)
			v1.position.z *= -1.0f;
			v2.position.z *= -1.0f;
			v3.position.z *= -1.0f;

			// Flip normal's Z
			v1.normal.z *= -1.0f;
			v2.normal.z *= -1.0f;
			v3.normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);
			vertCounter += 3;

			// Add three more indices
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;

			// Was there a 4th face?
			// - 12 numbers read means 4 faces WITH uv's
			// - 8 numbers read means 4 faces WITHOUT uv's
			if (numbersRead == 12 || numbersRead == 8)
			{
				// Make the last vertex
				Vertex v4;
				v4.position = positions[i[9] - 1];
				v4.uv = uvs[i[10] - 1];
				v4.normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal's Z
				v4.uv.y = 1.0f - v4.uv.y;
				v4.position.z *= -1.0f;
				v4.normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);
				vertCounter += 3;

				// Add three more indices
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	// - At this point, "verts" is a vector of Vertex structs, and can be used
	//    directly to create a vertex buffer:  &verts[0] is the address of the first vert
	//
	// - The vector "indices" is similar. It's a vector of unsigned ints and
	//    can be used directly for the index buffer: &indices[0] is the address of the first int
	//
	// - "vertCounter" is the number of vertices
	// - "indexCounter" is the number of indices
	// - Yes, these are effectively the same since OBJs do not index entire vertices!  This means
	//    an index buffer isn't doing much for us.  We could try to optimize the mesh ourselves
	//    and detect duplicate vertices, but at that point it would be better to use a more
	//    sophisticated model loading library like TinyOBJLoader or The Open Asset Importer Library

	this->vertexCount = vertCounter;
	this->indexCount = indexCounter;
	this->vertices = &verts[0];
	this->indices = &indices[0];

	CalculateTangents(&verts[0], vertCounter, &indices[0], indexCounter);
	CreateBuffers(&verts[0], vertCounter, &indices[0], indexCounter);
}
Mesh::~Mesh()
{
	
}

// --------------------------------------------------------
// Calculates the tangents of the vertices in a mesh
// - Code originally adapted from: http://www.terathon.com/code/tangent.html
// - Updated version found here: http://foundationsofgameenginedev.com/FGED2-sample.pdf
// - See listing 7.4 in section 7.5 (page 9 of the PDF)
//
// - Note: For this code to work, your Vertex format must
// contain an XMFLOAT3 called Tangent
//
// - Be sure to call this BEFORE creating your D3D vertex/index buffers
// --------------------------------------------------------
void Mesh::CalculateTangents(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount)
{
	// Reset tangents
	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].tangent = XMFLOAT3(0, 0, 0);
	}
	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < indexCount;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = indices[i++];
		unsigned int i2 = indices[i++];
		unsigned int i3 = indices[i++];
		Vertex* v1 = &vertices[i1];
		Vertex* v2 = &vertices[i2];
		Vertex* v3 = &vertices[i3];
		// Calculate vectors relative to triangle positions
		float x1 = v2->position.x - v1->position.x;
		float y1 = v2->position.y - v1->position.y;
		float z1 = v2->position.z - v1->position.z;
		float x2 = v3->position.x - v1->position.x;
		float y2 = v3->position.y - v1->position.y;
		float z2 = v3->position.z - v1->position.z;
		// Do the same for vectors relative to triangle uv's
		float s1 = v2->uv.x - v1->uv.x;
		float t1 = v2->uv.y - v1->uv.y;
		float s2 = v3->uv.x - v1->uv.x;
		float t2 = v3->uv.y - v1->uv.y;
		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);
		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;
		// Adjust tangents of each vert of the triangle
		v1->tangent.x += tx;
		v1->tangent.y += ty;
		v1->tangent.z += tz;
		v2->tangent.x += tx;
		v2->tangent.y += ty;
		v2->tangent.z += tz;
		v3->tangent.x += tx;
		v3->tangent.y += ty;
		v3->tangent.z += tz;
	}
	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < vertexCount; i++)
	{
		// Grab the two vectors
		XMVECTOR normal = XMLoadFloat3(&vertices[i].normal);
		XMVECTOR tangent = XMLoadFloat3(&vertices[i].tangent);
		// Use Gram-Schmidt orthonormalize to ensure
		// the normal and tangent are exactly 90 degrees apart
		tangent = XMVector3Normalize(
			tangent - normal * XMVector3Dot(normal, tangent));
		// Store the tangent
		XMStoreFloat3(&vertices[i].tangent, tangent);
	}
}

void Mesh::CreateBuffers(Vertex* vertices, int vertexCount, UINT* indices, int indexCount)
{
	this->vertices = vertices;
	this->indices = indices;
	this->vertexCount = vertexCount;
	this->indexCount = indexCount;

	/* Create Vertex Buffer */

	D3D11_BUFFER_DESC vbInfo;
	vbInfo.Usage = D3D11_USAGE_IMMUTABLE; // Buffer can't be modified
	vbInfo.ByteWidth = sizeof(Vertex) * vertexCount;
	vbInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbInfo.CPUAccessFlags = 0;
	vbInfo.MiscFlags = 0;
	vbInfo.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;

	Graphics::Device->CreateBuffer(&vbInfo, &initialVertexData, vertexBuffer.GetAddressOf());

	/* Create Index Buffer */

	D3D11_BUFFER_DESC ibInfo;
	ibInfo.Usage = D3D11_USAGE_IMMUTABLE; // Buffer can't be modified
	ibInfo.ByteWidth = sizeof(unsigned int) * indexCount;
	ibInfo.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibInfo.CPUAccessFlags = 0;
	ibInfo.MiscFlags = 0;
	ibInfo.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;

	Graphics::Device->CreateBuffer(&ibInfo, &initialIndexData, indexBuffer.GetAddressOf());
}

void Mesh::Draw()
{
	// Set vertex and index buffers to the ones used for this mesh
	UINT stride = sizeof(Vertex); // Space between starting indices for each vertex
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Start drawing the mesh
	Graphics::Context->DrawIndexed(indexCount, 0, 0);
}