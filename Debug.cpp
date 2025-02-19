#include "Debug.h"

#include "Mesh.h"
#include "DebugEntity.h"

namespace Debug
{
	namespace
	{
		std::shared_ptr<Material> debugMaterial;
		std::shared_ptr<Mesh> debugBoxMesh;

		std::vector<DebugMesh*> debugMeshes;

		std::vector<DebugMesh*> debugWireframes;
	}
}

void Debug::CreateDebugBox(Transform* transform, Vector offset, Vector halfSize)
{
	debugMeshes.push_back(new DebugBox(transform, debugBoxMesh, offset, halfSize));
}
void Debug::CreateDebugBox(Vector* location, Vector* rotation, Vector* scale)
{

}

void Debug::CreateWireframe_Temp(Vector location, Vector rotation, Vector halfSize)
{
	debugWireframes.push_back(new DebugBox(debugBoxMesh, location, rotation, halfSize));
}

void Debug::DrawAllWireframes(std::shared_ptr<Camera> camera)
{
	for(auto& e : debugMeshes)
	{
		Transform transform = e->GetTransform();

		std::shared_ptr<SimpleVertexShader> vs = debugMaterial->GetVertexShader();
		std::shared_ptr<SimplePixelShader> ps = debugMaterial->GetPixelShader();

		debugMaterial->GetVertexShader()->SetShader();
		debugMaterial->GetPixelShader()->SetShader();

		// Have the material set up the shader with its private values
		debugMaterial->PrepareMaterial();

		// Create data to be sent to the vertex shader
		vs->SetMatrix4x4("worldMatrix", transform.GetWorldMatrix());
		vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
		vs->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
		vs->SetMatrix4x4("projMatrix", camera->GetProjectionMatrix());

		ps->SetFloat3("cameraLocation", DirectX::XMFLOAT3(
			camera->GetTransform().GetLocation().x, 
			camera->GetTransform().GetLocation().y, 
			camera->GetTransform().GetLocation().z));

		vs->CopyAllBufferData();
		ps->CopyAllBufferData();

		e->mesh->Draw();
	}

	for(auto& e : debugWireframes)
	{
		Transform transform = e->GetTransform();

		std::shared_ptr<SimpleVertexShader> vs = debugMaterial->GetVertexShader();
		std::shared_ptr<SimplePixelShader> ps = debugMaterial->GetPixelShader();

		debugMaterial->GetVertexShader()->SetShader();
		debugMaterial->GetPixelShader()->SetShader();

		// Have the material set up the shader with its private values
		debugMaterial->PrepareMaterial();

		// Create data to be sent to the vertex shader
		vs->SetMatrix4x4("worldMatrix", transform.GetWorldMatrix());
		vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
		vs->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
		vs->SetMatrix4x4("projMatrix", camera->GetProjectionMatrix());

		ps->SetFloat3("cameraLocation", DirectX::XMFLOAT3(
			camera->GetTransform().GetLocation().x,
			camera->GetTransform().GetLocation().y,
			camera->GetTransform().GetLocation().z));

		vs->CopyAllBufferData();
		ps->CopyAllBufferData();

		e->mesh->Draw();
	}

	for(DebugMesh* m : debugWireframes)
		if(m)
			delete m;

	debugWireframes.clear();
}

void Debug::SetDebugMaterial(std::shared_ptr<Material> material)
{
	debugMaterial = material;
}
void Debug::SetDebugBoxMesh(std::shared_ptr<Mesh> mesh)
{
	debugBoxMesh = mesh;
}