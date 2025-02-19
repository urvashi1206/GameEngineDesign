#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include <memory>
#include <iostream>
#include "BoxCollider.h"
#include "Debug.h"

// From DirectX Tool Kit
#include "WICTextureLoader.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadTextures();
	LoadShaders();
	
	CreateShadowMap();
	CreateMaterials();
	CreateGeometry();
	CreateLights();
	InitializePostProcessEffects();

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME; // Wireframe mode
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthClipEnable = true;

	Graphics::Device->CreateRasterizerState(&rasterDesc, &wireframeState);

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	/* Create Camera */

	// First camera - this is the default active camera
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(0, 0, -1), XMFLOAT3(0, 0, 0), (float) Window::Width() / Window::Height(), 60.0f));

	// Low FOV camera from the left, looking 45 degrees to the right
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(-1, 0, -1), XMFLOAT3(0, XM_PIDIV4, 0), (float) Window::Width() / Window::Height(), 45.0f));
	// High FOV camera from the right, looking 45 degrees to the left
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(1, 0, -1), XMFLOAT3(0, -XM_PIDIV4, 0), (float) Window::Width() / Window::Height(), 90.0f));

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	ImGui::StyleColorsDark();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Clean up for ImGui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Game::LoadTextures()
{
	// Load all textures from their paths and associate them with identifiers in an unordered map
	for(std::wstring texturePath : texturePaths)
	{
		/* The overload that takes the device context also makes mipmaps */
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
		CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(texturePath).c_str(), 0, textureSRV.GetAddressOf());

		// Texture name is the final argument of the path (after the last /), i.e. "brokentiles.png"
		std::wstring textureName = texturePath.substr(texturePath.find_last_of('/') + 1);
		// Add the SRV to the map so it can be accessed by its texture name
		textureSRVs.insert({ textureName.c_str(), textureSRV });
	}

	// Load cubemaps from paths and add them to textureSRVs
	for(auto& cubemap : cubemapPaths)
		textureSRVs.insert({ cubemap.first, CreateCubemap(
			FixPath(cubemap.second[0]).c_str(),
			FixPath(cubemap.second[1]).c_str(),
			FixPath(cubemap.second[2]).c_str(),
			FixPath(cubemap.second[3]).c_str(),
			FixPath(cubemap.second[4]).c_str(),
			FixPath(cubemap.second[5]).c_str()) });

	// Create the basic texture sampler
	D3D11_SAMPLER_DESC samplerDesc = {};
	// Clamp coordinate values so texture stretches from extremes
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16; // Must be power of 2, 0-16 - higher is better

	Graphics::Device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	normalPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSNormal.cso").c_str());
	uvPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSUV.cso").c_str());
	customPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSCustom.cso").c_str());

	voxelPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSVoxel.cso").c_str());

	skyboxVertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VSSky.cso").c_str());
	skyboxPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSSky.cso").c_str());

	shadowVertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VSShadowMap.cso").c_str());

	postProcessVertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VSFullscreen.cso").c_str());
	postProcessBlurPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSBoxBlur.cso").c_str());
	postProcessAberrationPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSChromaticAberration.cso").c_str());
	postProcessPixelizationPixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PSPixelization.cso").c_str());
}

void Game::InitializePostProcessEffects()
{
	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	/* Post-Process Effect 1: Box Blur */

	// Create texture for rendering and sampling post-process effects
	D3D11_TEXTURE2D_DESC blurTextureDesc = {};
	blurTextureDesc.Width = Window::Width();
	blurTextureDesc.Height = Window::Height();
	blurTextureDesc.ArraySize = 1;
	blurTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	blurTextureDesc.CPUAccessFlags = 0;
	blurTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	blurTextureDesc.MipLevels = 1;
	blurTextureDesc.MiscFlags = 0;
	blurTextureDesc.SampleDesc.Count = 1;
	blurTextureDesc.SampleDesc.Quality = 0;
	blurTextureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppBlurTexture;
	Graphics::Device->CreateTexture2D(&blurTextureDesc, 0, ppBlurTexture.GetAddressOf());

	// Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvBlurDesc = {};
	rtvBlurDesc.Format = blurTextureDesc.Format;
	rtvBlurDesc.Texture2D.MipSlice = 0;
	rtvBlurDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(ppBlurTexture.Get(), &rtvBlurDesc, postProcessBlurRTV.ReleaseAndGetAddressOf());

	// Shader Resource View
	Graphics::Device->CreateShaderResourceView(ppBlurTexture.Get(), 0, postProcessBlurSRV.ReleaseAndGetAddressOf());

	/* Post-Process Effect 2: Chromatic Aberration */

	// Create texture for rendering and sampling post-process effects
	D3D11_TEXTURE2D_DESC aberrationTextureDesc = {};
	aberrationTextureDesc.Width = Window::Width();
	aberrationTextureDesc.Height = Window::Height();
	aberrationTextureDesc.ArraySize = 1;
	aberrationTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	aberrationTextureDesc.CPUAccessFlags = 0;
	aberrationTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	aberrationTextureDesc.MipLevels = 1;
	aberrationTextureDesc.MiscFlags = 0;
	aberrationTextureDesc.SampleDesc.Count = 1;
	aberrationTextureDesc.SampleDesc.Quality = 0;
	aberrationTextureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppAberrationTexture;
	Graphics::Device->CreateTexture2D(&aberrationTextureDesc, 0, ppAberrationTexture.GetAddressOf());

	// Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvAberrationDesc = {};
	rtvAberrationDesc.Format = aberrationTextureDesc.Format;
	rtvAberrationDesc.Texture2D.MipSlice = 0;
	rtvAberrationDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(ppAberrationTexture.Get(), &rtvAberrationDesc, postProcessAberrationRTV.ReleaseAndGetAddressOf());

	// Shader Resource View
	Graphics::Device->CreateShaderResourceView(ppAberrationTexture.Get(), 0, postProcessAberrationSRV.ReleaseAndGetAddressOf());

	/* Post-Process Effect 3: Pixelization */

	// Create texture for rendering and sampling post-process effects
	D3D11_TEXTURE2D_DESC pixelizationTextureDesc = {};
	pixelizationTextureDesc.Width = Window::Width();
	pixelizationTextureDesc.Height = Window::Height();
	pixelizationTextureDesc.ArraySize = 1;
	pixelizationTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	pixelizationTextureDesc.CPUAccessFlags = 0;
	pixelizationTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pixelizationTextureDesc.MipLevels = 1;
	pixelizationTextureDesc.MiscFlags = 0;
	pixelizationTextureDesc.SampleDesc.Count = 1;
	pixelizationTextureDesc.SampleDesc.Quality = 0;
	pixelizationTextureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppPixelizationTexture;
	Graphics::Device->CreateTexture2D(&pixelizationTextureDesc, 0, ppPixelizationTexture.GetAddressOf());

	// Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvPixelizationDesc = {};
	rtvPixelizationDesc.Format = pixelizationTextureDesc.Format;
	rtvPixelizationDesc.Texture2D.MipSlice = 0;
	rtvPixelizationDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(ppPixelizationTexture.Get(), &rtvPixelizationDesc, postProcessPixelizationRTV.ReleaseAndGetAddressOf());

	// Shader Resource View
	Graphics::Device->CreateShaderResourceView(ppPixelizationTexture.Get(), 0, postProcessPixelizationSRV.ReleaseAndGetAddressOf());
}

void Game::CreateMaterials()
{
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (bronze)
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (cobblestone)
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (floor)
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (paint)
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (rough)
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (scratched)
	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (wood)

	materials.push_back(std::make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), XMFLOAT2(5, 5))); // Floor material

	materials.push_back(std::make_shared<Material>(vertexShader, voxelPixelShader, XMFLOAT4(1, 1, 1, 1))); // Textured material (voxel)

	/* Materials with albedo, normals, roughness, metalness maps (PBR) */

	materials[0]->AddTextureSRV("AlbedoTexture", textureSRVs[L"bronze_albedo.png"]);
	materials[0]->AddTextureSRV("NormalMap", textureSRVs[L"bronze_normals.png"]);
	materials[0]->AddTextureSRV("RoughnessMap", textureSRVs[L"bronze_roughness.png"]);
	materials[0]->AddTextureSRV("MetalnessMap", textureSRVs[L"bronze_metal.png"]);
	materials[0]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[0]->AddSampler("BasicSampler", sampler);
	materials[0]->AddSampler("ShadowSampler", shadowSampler);

	materials[1]->AddTextureSRV("AlbedoTexture", textureSRVs[L"cobblestone_albedo.png"]);
	materials[1]->AddTextureSRV("NormalMap", textureSRVs[L"cobblestone_normals.png"]);
	materials[1]->AddTextureSRV("RoughnessMap", textureSRVs[L"cobblestone_roughness.png"]);
	materials[1]->AddTextureSRV("MetalnessMap", textureSRVs[L"cobblestone_metal.png"]);
	materials[1]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[1]->AddSampler("BasicSampler", sampler);
	materials[1]->AddSampler("ShadowSampler", shadowSampler);

	materials[2]->AddTextureSRV("AlbedoTexture", textureSRVs[L"floor_albedo.png"]);
	materials[2]->AddTextureSRV("NormalMap", textureSRVs[L"floor_normals.png"]);
	materials[2]->AddTextureSRV("RoughnessMap", textureSRVs[L"floor_roughness.png"]);
	materials[2]->AddTextureSRV("MetalnessMap", textureSRVs[L"floor_metal.png"]);
	materials[2]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[2]->AddSampler("BasicSampler", sampler);
	materials[2]->AddSampler("ShadowSampler", shadowSampler);

	materials[3]->AddTextureSRV("AlbedoTexture", textureSRVs[L"paint_albedo.png"]);
	materials[3]->AddTextureSRV("NormalMap", textureSRVs[L"paint_normals.png"]);
	materials[3]->AddTextureSRV("RoughnessMap", textureSRVs[L"paint_roughness.png"]);
	materials[3]->AddTextureSRV("MetalnessMap", textureSRVs[L"paint_metal.png"]);
	materials[3]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[3]->AddSampler("BasicSampler", sampler);
	materials[3]->AddSampler("ShadowSampler", shadowSampler);

	materials[4]->AddTextureSRV("AlbedoTexture", textureSRVs[L"rough_albedo.png"]);
	materials[4]->AddTextureSRV("NormalMap", textureSRVs[L"rough_normals.png"]);
	materials[4]->AddTextureSRV("RoughnessMap", textureSRVs[L"rough_roughness.png"]);
	materials[4]->AddTextureSRV("MetalnessMap", textureSRVs[L"rough_metal.png"]);
	materials[4]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[4]->AddSampler("BasicSampler", sampler);
	materials[4]->AddSampler("ShadowSampler", shadowSampler);

	materials[5]->AddTextureSRV("AlbedoTexture", textureSRVs[L"scratched_albedo.png"]);
	materials[5]->AddTextureSRV("NormalMap", textureSRVs[L"scratched_normals.png"]);
	materials[5]->AddTextureSRV("RoughnessMap", textureSRVs[L"scratched_roughness.png"]);
	materials[5]->AddTextureSRV("MetalnessMap", textureSRVs[L"scratched_metal.png"]);
	materials[5]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[5]->AddSampler("BasicSampler", sampler);
	materials[5]->AddSampler("ShadowSampler", shadowSampler);

	materials[6]->AddTextureSRV("AlbedoTexture", textureSRVs[L"wood_albedo.png"]);
	materials[6]->AddTextureSRV("NormalMap", textureSRVs[L"wood_normals.png"]);
	materials[6]->AddTextureSRV("RoughnessMap", textureSRVs[L"wood_roughness.png"]);
	materials[6]->AddTextureSRV("MetalnessMap", textureSRVs[L"wood_metal.png"]);
	materials[6]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[6]->AddSampler("BasicSampler", sampler);
	materials[6]->AddSampler("ShadowSampler", shadowSampler);

	materials[7]->AddTextureSRV("AlbedoTexture", textureSRVs[L"wood_albedo.png"]);
	materials[7]->AddTextureSRV("NormalMap", textureSRVs[L"wood_normals.png"]);
	materials[7]->AddTextureSRV("RoughnessMap", textureSRVs[L"wood_roughness.png"]);
	materials[7]->AddTextureSRV("MetalnessMap", textureSRVs[L"wood_metal.png"]);
	materials[7]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[7]->AddSampler("BasicSampler", sampler);
	materials[7]->AddSampler("ShadowSampler", shadowSampler);

	materials[8]->AddTextureSRV("AlbedoTexture", textureSRVs[L"KhaimBook.png"]);
	materials[8]->AddTextureSRV("NormalMap", textureSRVs[L"wood_normals.png"]);
	materials[8]->AddTextureSRV("RoughnessMap", textureSRVs[L"wood_roughness.png"]);
	materials[8]->AddTextureSRV("MetalnessMap", textureSRVs[L"wood_metal.png"]);
	materials[8]->AddTextureSRV("ShadowMap", shadowSRV);
	materials[8]->AddSampler("BasicSampler", sampler);
	materials[8]->AddSampler("ShadowSampler", shadowSampler);

	// Set debug material
	Debug::SetDebugMaterial(materials[8]);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	/* Import meshes from file */

	// Primitives
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str()));

	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/KhaimBook.obj").c_str()));

	/* Assign debug meshes */
	Debug::SetDebugBoxMesh(meshes[0]);

	/* Create skybox */
	skybox = std::make_shared<Skybox>(skyboxVertexShader, skyboxPixelShader, meshes[0], sampler, textureSRVs[L"Cold Sunset"]);

	/* Create entities */

	std::shared_ptr<Entity> floor = std::make_shared<Entity>(meshes[5], materials[7]);
	floor->GetTransform()->MoveAbsolute(Vector(0, -10, 0));
	floor->GetTransform()->Scale(Vector(5, 5, 5));
	entities.push_back(floor);

	physics.AddRigidbody(Rigidbody(floor->GetTransform(), new BoxCollider(floor->GetTransform(), Vector(0, 0, 0), Vector(5.0f, 0.25f, 5.0f), true), Vector(), true));

	std::shared_ptr<Entity> newEntity = std::make_shared<Entity>(meshes[7], materials[8]);
	newEntity->GetTransform()->MoveAbsolute(Vector(0, -1.0f, 0));
	newEntity->GetTransform()->SetRotation(Vector(0, 0, 0));
	newEntity->GetTransform()->Scale(Vector(0.5f, 0.5f, 0.5f));
	entities.push_back(newEntity);

	BoxCollider* collider = new BoxCollider(newEntity->GetTransform(), Vector(0, 0, 0), Vector(0.75f, 0.5f, 0.75f), true);
	physics.AddRigidbody(Rigidbody(newEntity->GetTransform(), collider, Vector(0, -9.81f, 0)));

	std::shared_ptr<Entity> newEntity2 = std::make_shared<Entity>(meshes[7], materials[8]);
	newEntity2->GetTransform()->MoveAbsolute(Vector(3, -1.0f, 0));
	newEntity2->GetTransform()->SetRotation(Vector(0, 0, 0));
	newEntity2->GetTransform()->Scale(Vector(0.5f, 0.5f, 0.5f));
	entities.push_back(newEntity2);

	BoxCollider* collider2 = new BoxCollider(newEntity2->GetTransform(), Vector(0, 0, 0), Vector(0.75f, 0.5f, 0.75f), true);
	physics.AddRigidbody(Rigidbody(newEntity2->GetTransform(), collider2));
}

void Game::CreateLights()
{
	/* Directional Light Settings */
	Light directionalLight1 = {};
	directionalLight1.LightType = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.Direction = DirectX::XMFLOAT3(1, -1, 0);
	directionalLight1.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	directionalLight1.Intensity = 1.0f;

	Light directionalLight2 = {};
	directionalLight2.LightType = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.Direction = DirectX::XMFLOAT3(0, 1, 0);
	directionalLight2.Color = DirectX::XMFLOAT3(1, 0.5f, 0);
	directionalLight2.Intensity = 1.0f;

	Light directionalLight3 = {};
	directionalLight3.LightType = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.Direction = DirectX::XMFLOAT3(-1, -1, 0);
	directionalLight3.Color = DirectX::XMFLOAT3(0, 1, 0);
	directionalLight3.Intensity = 0.5f;

	/* Point Light Settings */
	Light pointLight1 = {};
	pointLight1.LightType = LIGHT_TYPE_POINT;
	pointLight1.Location = DirectX::XMFLOAT3(0, -1, 5);
	pointLight1.Color = DirectX::XMFLOAT3(1, 0, 0);
	pointLight1.Intensity = 1.0f;
	pointLight1.Range = 4.0f;

	Light pointLight2 = {};
	pointLight2.LightType = LIGHT_TYPE_POINT;
	pointLight2.Location = DirectX::XMFLOAT3(2.5f, 0, 5);
	pointLight2.Color = DirectX::XMFLOAT3(0, 0, 1);
	pointLight2.Intensity = 1.0f;
	pointLight2.Range = 4.0f;

	lights.push_back(directionalLight1);
	/*lights.push_back(directionalLight2);
	lights.push_back(directionalLight3);
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);*/

	UpdateShadowMapMatrices(directionalLight1);
}

void Game::CreateShadowMap()
{
	// Create shadow map texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = 1024; // Ideally a power of 2
	shadowDesc.Height = 1024; // Ideally a power of 2
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapTexture; // Shadow texture is only used for creating SRV and DSV; can be local
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowMapTexture.GetAddressOf());

	// Create shadow map SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(shadowMapTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	// Create shadow map DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(shadowMapTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	// Create shadow map rasterizer
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);
	
	// Create shadow map sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
}

// --------------------------------------------------------
// Author: Chris Cascioli
// Purpose: Creates a cube map on the GPU from 6 individual textures
// 
// - You are allowed to directly copy/paste this into your code base
//   for assignments, given that you clearly cite that this is not
//   code of your own design.
//
// - Note: This code assumes you’re putting the function in Sky.cpp, 
//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
//   ComPtr called “device”.  Make any adjustments necessary for
//   your own implementation.
// --------------------------------------------------------

// --------------------------------------------------------
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::CreateCubemap(
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
	// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
	CreateWICTextureFromFile(Graphics::Device.Get(), right, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), left, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), up, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), down, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), front, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), back, (ID3D11Resource**)textures[5].GetAddressOf(), 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first texture
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array" with the TEXTURECUBE flag set.  
	// This is a special GPU resource format, NOT just a 
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6;            // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0;       // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;   // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1;            // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	Graphics::Device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, since there's only one)
			i,  // Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		Graphics::Context->CopySubresourceRegion(
			cubeMapTexture.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			textures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;         // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;        // Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0;  // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	Graphics::Device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for(std::shared_ptr<Camera> camera : cameras)
		camera->UpdateProjectionMatrix((float) Window::Width() / Window::Height());

	// Re-generate render targets used for post-processing
	if(Graphics::Device)
		UpdatePostProcessRenderTargets();
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	this->totalTime = totalTime;

	UpdateImGui(deltaTime, totalTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	BuildUI();

	physics.Update(deltaTime);

	GetCamera()->Update(deltaTime);
}

void Game::UpdateImGui(float deltaTime, float totalTime) const
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	if (!isDemoWindowHidden)
		ImGui::ShowDemoWindow();
}

void Game::BuildUI()
{
	ImGui::Begin("Custom Window");

	ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
	ImGui::Text("Window Size: %ix%i", Window::Width(), Window::Height());
	ImGui::ColorEdit4("Background Color", backgroundColor);

	// Button to toggle visibility of the ImGui demo window
	if(ImGui::Button("Toggle Demo Window"))
		isDemoWindowHidden = !isDemoWindowHidden;

	static Vector supportDirection;
	ImGui::DragFloat3("Support Direction", &supportDirection.x, 0.1f, -1, 1);
	Vector supportPoint = ((ConvexCollider*) physics.GetRigidbody(1).GetCollider())->GJK_Support(supportDirection);
	Debug::CreateWireframe_Temp(supportPoint);

	if(ImGui::TreeNode("Post-Process Effects"))
	{
		ImGui::DragInt("Blur", &postProcessBlurAmount, 0.1f, 0, 100);
		ImGui::DragFloat3("Aberration", &postProcessAberrationAmount.x, 0.0001f);
		ImGui::DragInt("Pixelization", &postProcessPixelizeAmount, 0.1f, 1, 100);

		ImGui::Text("Before Post-Processing:");
		ImGui::Image(postProcessBlurSRV.Get(), ImVec2(256, 256));

		ImGui::Text("After Box Blur:");
		ImGui::Image(postProcessAberrationSRV.Get(), ImVec2(256, 256));

		ImGui::Text("After Box Blur and Chromatic Aberration:");
		ImGui::Image(postProcessPixelizationSRV.Get(), ImVec2(256, 256));

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Shadows"))
	{
		ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Lights"))
	{
		for(int i = 0; i < lights.size(); i++)
		{
			ImGui::PushID(&lights[i]);
			if(ImGui::TreeNode("Light", "Light %i", i))
			{
				ImGui::DragInt("Light Type (0: Dir, 1: Point, 2: Spot)", &lights[i].LightType, 0.1f, 0, 2);

				ImGui::DragFloat3("Location", &lights[i].Location.x, 0.01f, -10, 10);
				ImGui::DragFloat3("Color", &lights[i].Color.x, 0.01f, 0, 1);
				ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f, -1, 1);
				ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.01f, 0, 10);
				ImGui::DragFloat("Range", &lights[i].Range, 0.01f, 0, 10);

				// If modifying the main directional light, update shadowmap matrices using the new values
				if(i == 0 && lights[i].LightType == LIGHT_TYPE_DIRECTIONAL)
					UpdateShadowMapMatrices(lights[i]);

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// Slider control for setting the active camera
	int index = activeCameraIndex;
	ImGui::DragInt("Camera Index", &index, 0.01f, 0, (int) cameras.size() - 1);
	activeCameraIndex = index;

	// Active camera information
	Vector location = GetCamera()->GetTransform().GetLocation();
	ImGui::Text("Location: (%.2f, %.2f, %.2f)", location.x, location.y, location.z);
	Vector rotation = GetCamera()->GetTransform().GetPitchYawRoll();
	ImGui::Text("Rotation (Radians): (%.2f, %.2f, %.2f)", rotation.x, rotation.y, rotation.z);
	ImGui::Text("FOV (Degrees): %.1f", GetCamera()->GetFOV());

	if(ImGui::TreeNode("Meshes"))
	{
		// Display the vertex and triangle count of each mesh
		for(std::shared_ptr<Mesh> mesh : meshes)
		{
			if(ImGui::TreeNode(mesh->GetName().c_str()))
			{
				ImGui::Text("Vertices: %i", mesh->GetVertexCount());
				ImGui::Text("Triangles: %i", mesh->GetIndexCount() / 3);
				ImGui::Text("Indices: %i", mesh->GetIndexCount());
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if(ImGui::TreeNode("Entities"))
	{
		// Display the transform information for each entity
		for(int i = 0; i < entities.size(); i++)
		{
			std::shared_ptr<Entity> entity = entities[i];

			ImGui::PushID(entity.get());
			if(ImGui::TreeNode("Entity", "Entity %i", i))
			{
				ImGui::Text("Mesh: %s", entity->GetMesh()->GetName().c_str());

				Transform* transform = entity->GetTransform();

				Vector location = transform->GetLocation();
				Vector rotation = transform->GetPitchYawRoll();
				Vector scale = transform->GetScale();

				if(ImGui::DragFloat3("Location", &location.x, 0.1f))
					transform->SetLocation(location);
				if(ImGui::DragFloat3("Rotation", &rotation.x, 0.25f))
					transform->SetRotation(rotation);
				if(ImGui::DragFloat3("Scale", &scale.x, 0.1f))
					transform->SetScale(scale);

				ImGui::Text("Mesh Index Count: %i", entity->GetMesh()->GetIndexCount());

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	ImGui::End();
}

void Game::UpdatePostProcessRenderTargets()
{
	// Reset all relevant pointers
	postProcessBlurRTV.Reset();
	postProcessBlurSRV.Reset();
	postProcessAberrationRTV.Reset();
	postProcessAberrationSRV.Reset();
	postProcessPixelizationRTV.Reset();
	postProcessPixelizationSRV.Reset();

	// Re-generate resources
	InitializePostProcessEffects();
}

void Game::UpdateShadowMapMatrices(Light directionalLight)
{
	XMVECTOR lightDirection = XMLoadFloat3(&directionalLight.Direction);
	XMMATRIX lightView = XMMatrixLookToLH(
		-lightDirection * 20, // Position: "Backing up" 20 units from origin
		lightDirection, // Direction: light's direction
		XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)

	float lightProjectionSize = 15.0f; // Tweak for your scene!
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);

	XMStoreFloat4x4(&shadowViewMatrix, lightView);
	XMStoreFloat4x4(&shadowProjectionMatrix, lightProjection);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		Graphics::Context->ClearRenderTargetView(postProcessBlurRTV.Get(), backgroundColor);

		/* Shadow mapping setup */

		Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		ID3D11RenderTargetView* nullRTV{};
		Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

		Graphics::Context->PSSetShader(0, 0, 0);

		Graphics::Context->RSSetState(shadowRasterizer.Get());

		D3D11_VIEWPORT viewport = {};
		viewport.Width = 1024.0f;
		viewport.Height = 1024.0f;
		viewport.MaxDepth = 1.0f;
		Graphics::Context->RSSetViewports(1, &viewport);

		shadowVertexShader->SetShader();
		shadowVertexShader->SetMatrix4x4("view", shadowViewMatrix);
		shadowVertexShader->SetMatrix4x4("projection", shadowProjectionMatrix);
		// Loop and draw all entities
		for(auto& e : entities)
		{
			shadowVertexShader->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
			shadowVertexShader->CopyAllBufferData();

			// Draw the mesh directly to avoid the entity's material
			e->GetMesh()->Draw();
		}

		Graphics::Context->RSSetState(0);

		viewport.Width = (float) Window::Width();
		viewport.Height = (float) Window::Height();
		Graphics::Context->RSSetViewports(1, &viewport);
		// Start draw by setting render target to first post-process step (results are processed by blur first before sent to back buffer)
		Graphics::Context->OMSetRenderTargets(1, postProcessBlurRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		// Draw all entities
		for(std::shared_ptr<Entity> e : entities)
		{
			// Give light data for main (shadow-casting) directional light
			e->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", shadowViewMatrix);
			e->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProj", shadowProjectionMatrix);

			// Give all light data
			e->GetMaterial()->GetPixelShader()->SetInt("lightCount", (int) lights.size());
			e->GetMaterial()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * (int) lights.size());

			e->Draw(GetCamera(), totalTime);
		}

		// Draw wireframes
		Graphics::Context->RSSetState(wireframeState.Get());

		//Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		for(std::shared_ptr<Entity> e : wireframes)
		{
			e->Draw(GetCamera(), totalTime);
		}

		Debug::DrawAllWireframes(cameras[activeCameraIndex]);

		Graphics::Context->RSSetState(0);

		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Draw skybox
		skybox->Draw(GetCamera());
	}

	/* POST-PROCESS */
	{
		/* Box Blur */

		// Render to next post-process step
		Graphics::Context->OMSetRenderTargets(1, postProcessAberrationRTV.GetAddressOf(), 0);

		postProcessVertexShader->SetShader();
		postProcessBlurPixelShader->SetShader();
		postProcessBlurPixelShader->SetShaderResourceView("Pixels", postProcessBlurSRV.Get());
		postProcessBlurPixelShader->SetSamplerState("Sampler", ppSampler.Get());

		postProcessBlurPixelShader->SetInt("blurRadius", postProcessBlurAmount);
		postProcessBlurPixelShader->SetFloat("pixelWidth", 1.0f / Window::Width());
		postProcessBlurPixelShader->SetFloat("pixelHeight", 1.0f / Window::Height());

		postProcessBlurPixelShader->CopyAllBufferData();

		Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one fullscreen triangle)

		/* Chromatic Aberration */

		// Render to next post-process step
		Graphics::Context->OMSetRenderTargets(1, postProcessPixelizationRTV.GetAddressOf(), 0);

		postProcessVertexShader->SetShader();
		postProcessAberrationPixelShader->SetShader();
		postProcessAberrationPixelShader->SetShaderResourceView("Pixels", postProcessAberrationSRV.Get());
		postProcessAberrationPixelShader->SetSamplerState("Sampler", ppSampler.Get());

		postProcessAberrationPixelShader->SetFloat2("mouseFocusPoint", { 0, 0 });
		postProcessAberrationPixelShader->SetFloat("redOffset", postProcessAberrationAmount.x);
		postProcessAberrationPixelShader->SetFloat("greenOffset", postProcessAberrationAmount.y);
		postProcessAberrationPixelShader->SetFloat("blueOffset", postProcessAberrationAmount.z);

		postProcessAberrationPixelShader->CopyAllBufferData();

		Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one fullscreen triangle)

		/* Pixelation */

		// Render to the back buffer this time, since it's the last post-process step
		Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

		postProcessVertexShader->SetShader();
		postProcessPixelizationPixelShader->SetShader();
		postProcessPixelizationPixelShader->SetShaderResourceView("Pixels", postProcessPixelizationSRV.Get());
		postProcessPixelizationPixelShader->SetSamplerState("Sampler", ppSampler.Get());

		postProcessPixelizationPixelShader->SetInt("pixelSize", postProcessPixelizeAmount);

		postProcessPixelizationPixelShader->CopyAllBufferData();

		Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one fullscreen triangle)
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ImGui::Render(); // Turn's this frame's UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());

		// Unbind ALL SRVs
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}
}



