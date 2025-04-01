#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Skybox
{
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

public:
	Skybox(
		std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader,
		std::shared_ptr<Mesh> skyMesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapSRV);
	~Skybox();

	void Draw(std::shared_ptr<Camera> camera);
};