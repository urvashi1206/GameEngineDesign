#pragma once

#include "ConvexCollider.h"

class BoxCollider : public ConvexCollider
{
private:
	Vector center;
	Vector halfSize;
	bool showDebug;

	Vector localMin;
	Vector localMax;

public:
	BoxCollider();
	BoxCollider(Vector center, Vector halfSize, bool showDebug = false);
	~BoxCollider();

	//void CreateDebugWireframe(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) const override;

	virtual Vector GJK_Support(const Vector& direction) const override;
	virtual std::vector<Vector> EPA_GetAlignedFace(const Vector& direction, Vector& out_faceNormal) const override;

protected:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

public:
	virtual Matrix4x4 GetInertiaTensor(float mass) const override;

	Vector GetCenter() const override { return GetTransform()->LocalToWorld_Point(center); };
	Vector GetHalfSize() const { return GetTransform()->LocalToWorld_Point(halfSize); };

	Vector GetWorldMin() const;
	Vector GetWorldMax() const;

	//Transform GetWorldMaxT() const { return Transform(transform->LocalToWorld(center + halfSize),); };

	//Entity* GetDebugWireframe() const override;
};