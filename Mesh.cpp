#include "rapidobj/rapidobj.hpp"

#include "Mesh.h"
#include "Graphics.h"
#include "PathHelpers.h"

#include <iostream>
#include <fstream>

#include <DirectXMath.h>


using namespace DirectX;

Mesh::Mesh(std::string name, std::vector<Vertex> vertices, std::vector<UINT> indices) : 
	name(name), vertices(vertices), indices(indices)
{
	CalculateTangents(vertices, indices);
	CreateBuffers(vertices, indices);
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

	//CalculateTangents(&vertices1[0], vertexCount, &indices1[0], indexCount);
	CreateBuffers(vertices1, indices1);
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
void Mesh::CalculateTangents(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
	// Reset tangents
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].tangent = XMFLOAT3(0, 0, 0);
	}
	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < indices.size();)
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
	for (int i = 0; i < vertices.size(); i++)
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

void Mesh::CreateBuffers(std::vector<Vertex> vertices, std::vector<UINT> indices)
{
	this->vertices = vertices;
	this->indices = indices;

	/* Create Vertex Buffer */

	D3D11_BUFFER_DESC vbInfo;
	vbInfo.Usage = D3D11_USAGE_IMMUTABLE; // Buffer can't be modified
	vbInfo.ByteWidth = sizeof(Vertex) * vertices.size();
	vbInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbInfo.CPUAccessFlags = 0;
	vbInfo.MiscFlags = 0;
	vbInfo.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = &vertices[0];

	Graphics::Device->CreateBuffer(&vbInfo, &initialVertexData, vertexBuffer.GetAddressOf());

	/* Create Index Buffer */

	D3D11_BUFFER_DESC ibInfo;
	ibInfo.Usage = D3D11_USAGE_IMMUTABLE; // Buffer can't be modified
	ibInfo.ByteWidth = sizeof(unsigned int) * indices.size();
	ibInfo.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibInfo.CPUAccessFlags = 0;
	ibInfo.MiscFlags = 0;
	ibInfo.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = &indices[0];

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
	Graphics::Context->DrawIndexed(indices.size(), 0, 0);
}