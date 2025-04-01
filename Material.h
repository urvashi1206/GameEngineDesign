#pragma once

#include <d3d11.h>
#include <memory>

#include <DirectXMath.h>

#include "SimpleShader.h"

class Material
{
private:
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

	DirectX::XMFLOAT4 color;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

public:
	Material(std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, 
		DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));
	~Material();

	void PrepareMaterial();

	void AddTextureSRV(std::string identifier, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string identifier, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	std::shared_ptr<SimpleVertexShader> GetVertexShader() { return vertexShader; };
	std::shared_ptr<SimplePixelShader> GetPixelShader() { return pixelShader; };
	DirectX::XMFLOAT4 GetColor() const { return color; };

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> value) { vertexShader = value; };
	void SetPixelShader(std::shared_ptr<SimplePixelShader> value) { pixelShader = value; };
	void SetColor(DirectX::XMFLOAT4 value) { color = value; };
	void SetUVScale(DirectX::XMFLOAT2 value) { uvScale = value; };
	void SetUVOffset(DirectX::XMFLOAT2 value) { uvOffset = value; };
};