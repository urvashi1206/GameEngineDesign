#include "ResourceManager.h"

#include <filesystem>
#include <iostream>

#include "Graphics.h"
#include "PathHelpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// From DirectX Tool Kit
#include "WICTextureLoader.h"

namespace
{
	ResourceManager* Instance;
}

ResourceManager::ResourceManager()
{

}
ResourceManager::~ResourceManager()
{

}

void ResourceManager::Startup()
{
	Instance = new ResourceManager();

    LoadAllTextures();
}
void ResourceManager::Shutdown()
{
	for(VoxelGrid* voxelGrid : Instance->voxelGrids)
	{
		if(voxelGrid) delete voxelGrid;
		voxelGrid = nullptr;
	}
	Instance->voxelGrids.clear();

	delete Instance;
}

void ResourceManager::LoadAllTextures()
{
    const std::wstring TEXTURES_PATH = L"../../Assets/Textures";
    const std::vector<std::wstring> VALID_TYPES = { L".png" };
    std::unordered_map<std::wstring, std::wstring> texturePaths = GetAllFilesInDirectory(TEXTURES_PATH, VALID_TYPES);
    
    // Load all textures from their paths and associate them with identifiers in an unordered map
	for(auto& texturePath : texturePaths)
        Instance->textureSRVs[texturePath.second] = LoadTexture(TEXTURES_PATH + L"/" + texturePath.first);
}

VoxelGrid* ResourceManager::LoadVoxelGrid(const char* filePath, size_t slices, const char* paletteFilePath)
{
	int width, height, channels;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 4); // Force 4 channels (RGBA)
    
    if(!data)
    {
        std::cerr << "Failed to load image!\n";
        return nullptr;
    }

    VoxelGrid* grid = new VoxelGrid(width, slices, (size_t) height / slices);

    // Load palette texture
    {
        int paletteWidth, paletteHeight, paletteChannels;
        unsigned char* paletteData = stbi_load(paletteFilePath, &paletteWidth, &paletteHeight, &paletteChannels, 4);
        int paletteSize = paletteWidth * paletteHeight;

        for(int i = 0; i < paletteSize * 4; i += 4)
            grid->palette.emplace(Color(paletteData[i], paletteData[i + 1], paletteData[i + 2], paletteData[i + 3]), Vector((float) i / (paletteSize * 4), 0.5f));

        stbi_image_free(paletteData);
    }

    for(int i = 0; i < grid->voxelCount; i++)
    {
        grid->voxelStates[i] = BoolVoxel(false);
        grid->voxelColors[i] = ColorVoxel(Vector());
    }

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 4;
            uint8_t r = data[index];
            uint8_t g = data[index + 1];
            uint8_t b = data[index + 2];
            uint8_t a = data[index + 3];

            /*std::cout << "Pixel (" << x << ", " << y << ") - RGBA("
                      << (int) r << ", " << (int) g << ", " << (int) b << ", " << (int) a << ")\n";*/

            grid->voxelStates[x + y * width] = BoolVoxel(a > 0);
            // If alpha isn't positive, use default vector; voxel counts as empty
            grid->voxelColors[x + y * width] = ColorVoxel((a > 0) ? grid->palette[Color(r, g, b, a)] : Vector());
        }
    }

    stbi_image_free(data);

    Instance->voxelGrids.push_back(grid);

	return grid;
}

void ResourceManager::AddVoxelGrid(VoxelGrid* voxelGrid)
{
	Instance->voxelGrids.push_back(voxelGrid);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ResourceManager::GetTexture(std::wstring identifier)
{
    return Instance->textureSRVs[identifier];
}
Material* ResourceManager::GetMaterialByID(std::wstring identifier)
{
	return Instance->materials[identifier];
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ResourceManager::LoadTexture(std::wstring texturePath)
{
    /* The overload that takes the device context also makes mipmaps */
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
    DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(texturePath).c_str(), 0, textureSRV.GetAddressOf());

    return textureSRV;
}

std::unordered_map<std::wstring, std::wstring> ResourceManager::GetAllFilesInDirectory(std::wstring directory, std::vector<std::wstring> extensions)
{
    std::wstring fullDirectory = FixPath(directory);

    std::unordered_map<std::wstring, std::wstring> identifiers;

    for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(fullDirectory))
    {
        if(std::filesystem::is_regular_file(entry))
        {
            std::wstring filePath = entry.path();
            size_t directoryEnd = filePath.find(fullDirectory) + fullDirectory.length();
            size_t extensionStart = filePath.find_last_of('.');

            std::wstring localPath = filePath.substr(directoryEnd + 1);
            std::wstring identifier = filePath.substr(directoryEnd + 1, extensionStart - (directoryEnd + 1));
            std::wstring extension = filePath.substr(extensionStart);

            if(std::find(extensions.begin(), extensions.end(), extension) != extensions.end())
                identifiers.emplace(localPath, identifier);

            //for(char c : identifier)
                //std::cout << c;
            //std::cout << std::endl;
        }
    }

    return identifiers;
}