#include "Material.h"

#include <iostream>

using namespace DirectX;

Material::Material(std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, 
	XMFLOAT4 color, XMFLOAT2 uvScale, XMFLOAT2 uvOffset) :
	vertexShader(vertexShader), pixelShader(pixelShader), color(color), uvScale(uvScale), uvOffset(uvOffset)
{
	
}
Material::~Material()
{

}

void Material::PrepareMaterial()
{
	// Set all texture SRVs and samplers on the pixel shader
	for(auto& srv : textureSRVs)
		pixelShader->SetShaderResourceView(srv.first.c_str(), srv.second);
	for(auto& sampler : samplers)
		pixelShader->SetSamplerState(sampler.first.c_str(), sampler.second);

	// Set constant buffer variables
	pixelShader->SetFloat4("colorTint", color);
	pixelShader->SetFloat2("uvScale", uvScale);
	pixelShader->SetFloat2("uvOffset", uvOffset);
}

void Material::AddTextureSRV(std::string identifier, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({identifier, srv});
}
void Material::AddSampler(std::string identifier, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({identifier, sampler});
}