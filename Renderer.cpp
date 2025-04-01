#include "Renderer.h"

#include "Graphics.h"
#include "Entity.h"

using namespace DirectX;

Renderer::Renderer(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) :
	mesh(mesh), material(material)
{

}
Renderer::~Renderer()
{
	
}

void Renderer::Draw(std::shared_ptr<Camera> camera, float totalTime)
{
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Have the material set up the shader with its private values
	material->PrepareMaterial();

	Transform* transform = entity->GetComponent<Transform>();

	// Create data to be sent to the vertex shader
	vs->SetMatrix4x4("worldMatrix", transform->GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	vs->SetMatrix4x4("projMatrix", camera->GetProjectionMatrix());

	ps->SetFloat3("cameraLocation", DirectX::XMFLOAT3(
		camera->GetTransform().GetLocation().x,
		camera->GetTransform().GetLocation().y,
		camera->GetTransform().GetLocation().z));
	ps->SetFloat("totalTime", totalTime);

	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	mesh->Draw();
}

void Renderer::Initialize()
{

}
void Renderer::Update(float deltaTime)
{

}