#pragma once

#include "Transform.h"
#include "Input.h"

class Camera
{
private:
	Transform transform;

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	float aspectRatio;
	float fov;
	float nearDistance = 0.1f;
	float farDistance = 100.0f;

	float movementSpeed = 5.0f;
	float lookSpeed = 0.0075f;

public:
	Camera(Vector location, Vector rotation, float aspectRatio, float fov);

	void Update(float deltaTime);

	DirectX::XMFLOAT4X4 UpdateViewMatrix();
	DirectX::XMFLOAT4X4 UpdateProjectionMatrix(float aspectRatio);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	Transform GetTransform() { return transform; }
	float GetFOV() { return fov; }
};