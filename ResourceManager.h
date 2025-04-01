#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "SimpleShader.h"
#include "VoxelGrid.h"
#include "Skybox.h"

class ResourceManager
{
private:
	std::vector<VoxelGrid*> voxelGrids;

	// Shaders and shader-related constructs
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> skyboxVertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> skyboxPixelShader;
	std::shared_ptr<SimplePixelShader> normalPixelShader;
	std::shared_ptr<SimplePixelShader> uvPixelShader;
	std::shared_ptr<SimplePixelShader> customPixelShader;

	std::shared_ptr<SimplePixelShader> voxelPixelShader;

	std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::wstring, Mesh*> meshes;
	std::unordered_map<std::wstring, Material*> materials;

	std::shared_ptr<Skybox> skybox;

private:
	ResourceManager();
	~ResourceManager();

public:
	static void Startup();
	static void Shutdown();

	static void LoadAllTextures();

	static VoxelGrid* LoadVoxelGrid(const char* filePath, size_t slices, const char* paletteFilePath);

	static void AddVoxelGrid(VoxelGrid* voxelGrid);

	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture(std::wstring identifier);
	static Material* GetMaterialByID(std::wstring identifier);

private:
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(std::wstring texturePath);

	static std::unordered_map<std::wstring, std::wstring> GetAllFilesInDirectory(std::wstring directory, std::vector<std::wstring> extensions);
};