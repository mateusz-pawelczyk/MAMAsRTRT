#pragma once

#include <DirectXMath.h>
#include <Windows.h>


class FPSCamera
{
public:
	FPSCamera();
	FPSCamera(DirectX::XMFLOAT3 m_position, DirectX::XMFLOAT3 m_forward, DirectX::XMFLOAT3 m_right);

	// Initialize camera properties
	void Init(float fovY, float aspectRatio, float nearPlane, float farPlane);

	// Update the camera's position and orientation based on input
	void Update(float* frameIndex, bool* cameraMoved);

	// Handle keyboard input
	void OnKeyDown(UINT8 wParam);
	void OnKeyUp(UINT8 wParam);

	// Handle mouse input
	void OnMouseMove(int deltaX, int deltaY);

	// Set camera position
	void SetPosition(const DirectX::XMFLOAT3& position);

	// Set camera rotation
	void SetRotation(float pitch, float yaw);

	void SetDeltaTime(float deltaTime);

	// Get the view matrix
	DirectX::XMMATRIX GetViewMatrix() const;

	// Get the projection matrix
	DirectX::XMMATRIX GetProjectionMatrix() const;

private:
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_forward;
	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_rotation; // Pitch, Yaw, Roll
	float m_fovY;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;
	float m_deltaTime = 0;

	bool m_moveForward = false;
	bool m_moveRight = false;
	bool m_moveLeft = false;
	bool m_moveBackward = false;
	bool m_moveUp = false;
	bool m_moveDown= false;

	int m_lastMouseX = 0;
	int m_lastMouseY = 0;

	// Update the view matrix
	void UpdateViewMatrix();

	// Update the projection matrix
	void UpdateProjectionMatrix();
};
