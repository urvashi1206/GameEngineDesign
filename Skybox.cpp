#include "Skybox.h"
#include "Graphics.h"

Skybox::Skybox(
	std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, 
	std::shared_ptr<Mesh> skyMesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapSRV) :
	vertexShader(vertexShader), pixelShader(pixelShader), skyMesh(skyMesh), sampler(sampler), cubeMapSRV(cubeMapSRV)
{
	/* Create Rasterizer State */

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT; // Render the backs of the mesh (inside of cube)

	Graphics::Device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

	/* Create Depth Stencil State */

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // Override pixels that have less or equal depth to current pixel so skybox isn't automatically culled

	Graphics::Device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
}
Skybox::~Skybox()
{

}

void Skybox::Draw(std::shared_ptr<Camera> camera)
{
	// Set render states
	Graphics::Context->RSSetState(rasterizerState.Get());
	Graphics::Context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	/* Set all shader data and draw mesh */

	vertexShader->SetShader();
	pixelShader->SetShader();

	vertexShader->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projectionMatrix", camera->GetProjectionMatrix());
	vertexShader->CopyAllBufferData();

	pixelShader->SetSamplerState("SkySampler", sampler);
	pixelShader->SetShaderResourceView("SkyCubeMap", cubeMapSRV);

	skyMesh->Draw();

	// Reset render states
	Graphics::Context->RSSetState(nullptr);
	Graphics::Context->OMSetDepthStencilState(nullptr, 0);
}