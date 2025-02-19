#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include "Vertex.h"

class Mesh
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Vertex* vertices;
	UINT vertexCount;
	UINT* indices;
	UINT indexCount;
	std::string name;

public:
	Mesh(std::string name, UINT vertexCount, Vertex vertices[], UINT indexCount, UINT indices[]);
	Mesh(const wchar_t* filePath);
	~Mesh();

	void CalculateTangents(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount);

	void CreateBuffers(Vertex* vertices, int vertexCount, UINT* indices, int indexCount);

	void Draw();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; };
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return indexBuffer; };
	UINT GetVertexCount() { return vertexCount; };
	UINT GetIndexCount() { return indexCount; };
	std::string GetName() { return name; };
};