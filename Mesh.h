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
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	std::string name;

public:
	Mesh(std::string name, std::vector<Vertex> vertices, std::vector<UINT> indices);
	Mesh(const wchar_t* filePath);
	~Mesh();

	void CalculateTangents(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

	void CreateBuffers(std::vector<Vertex> vertices, std::vector<UINT> indices);

	void Draw();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; };
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return indexBuffer; };
	UINT GetVertexCount() { return vertices.size(); };
	UINT GetIndexCount() { return indices.size(); };
	std::string GetName() { return name; };
};