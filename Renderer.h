#pragma once

#include "Component.h"

#include <memory>

#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class Renderer : public Component
{
protected:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

public:
	Renderer(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Renderer();

	void Draw(std::shared_ptr<Camera> camera, float totalTime);

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

public:
	std::shared_ptr<Mesh> GetMesh() const { return mesh; }
	std::shared_ptr<Material> GetMaterial() const { return material; };

	void SetMaterial(std::shared_ptr<Material> value) { material = value; };
};