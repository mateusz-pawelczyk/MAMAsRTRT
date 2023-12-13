#include "stdafx.h"
#include "FPSCamera.h"

FPSCamera::FPSCamera()
	: m_position(0.0f, 0.0f, 0.0f),      // Initialize position at the origin
	m_forward(0.0f, 0.0f, 1.0f),
	m_right(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f),      // Initialize rotation (pitch, yaw, roll) to zero
	m_fovY(DirectX::XM_PIDIV4),                 // Default field of view is 45 degrees (PI/4)
	m_aspectRatio(1.0f),               // Default aspect ratio (should be set properly later)
	m_nearPlane(0.1f),                 // Default near plane
	m_farPlane(1000.0f)                // Default far plane
{
	// Initialize the view matrix and the projection matrix.
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

FPSCamera::FPSCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 forward, DirectX::XMFLOAT3 right)
	: m_position(position),      
	m_forward(forward),
	m_right(right),
	m_rotation(0.0f, 0.0f, 0.0f),      // Initialize rotation (pitch, yaw, roll) to zero
	m_fovY(DirectX::XM_PIDIV4),                 // Default field of view is 45 degrees (PI/4)
	m_aspectRatio(1.0f),               // Default aspect ratio (should be set properly later)
	m_nearPlane(0.1f),                 // Default near plane
	m_farPlane(1000.0f)                // Default far plane
{
	// Initialize the view matrix and the projection matrix.
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void FPSCamera::UpdateViewMatrix()
{
	using namespace DirectX;

	// Convert rotation angles (pitch and yaw) to radians and ensure roll is zero for an FPS camera
	float pitch = XMConvertToRadians(m_rotation.x);
	float yaw = XMConvertToRadians(m_rotation.y);
	float roll = 0.0f; // Typically, roll is not used in an FPS camera

	// Calculate the direction the camera is looking at
	XMVECTOR forward = XMVectorSet(
		cosf(pitch) * sinf(yaw),
		sinf(pitch),
		cosf(pitch) * cosf(yaw),
		0.0f);

	// Define the world's up direction
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Calculate the camera's right and up vectors
	XMVECTOR right = XMVector3Cross(forward, worldUp);
	XMVECTOR up = XMVector3Cross(right, forward);


	// Calculate the view matrix
	XMVECTOR eyePosition = XMLoadFloat3(&m_position);
	XMVECTOR lookAtPosition = eyePosition + forward;

	// Store the normalized right and forward vectors in your member variables
	XMStoreFloat3(&m_right, XMVector3Normalize(right));
	XMStoreFloat3(&m_forward, XMVector3Normalize(forward));

	XMMATRIX view = XMMatrixLookAtLH(eyePosition, lookAtPosition, up);

	// Store the view matrix
	m_viewMatrix = view;
}



void FPSCamera::UpdateProjectionMatrix()
{
	using namespace DirectX;

	// Calculate the projection matrix using field of view, aspect ratio, and near/far planes.
	// XMMatrixPerspectiveFovLH creates a left-handed perspective projection matrix.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(m_fovY), // Convert field of view from degrees to radians
		m_aspectRatio,
		m_nearPlane,
		m_farPlane
	);
}

// Define movement speed (units per second)
const float MovementSpeed = .1f;

void FPSCamera::OnKeyDown(UINT8 wParam)
{
	using namespace DirectX;

	switch (wParam)
	{
	case 'W': // Move forward
		m_moveForward = true;
		break;
	case 'S': // Move backward
		m_moveBackward = true;
		break;
	case 'A': // Move left (strafe)
		m_moveLeft = true;
		break;
	case 'D': // Move right (strafe)
		m_moveRight = true;

		break;
	case VK_SPACE: // Move up
		m_moveUp = true;
		break;
	case VK_SHIFT: // Move up
		m_moveDown = true;
		break;
	}


}
void FPSCamera::OnKeyUp(UINT8 wParam)
{
	using namespace DirectX;

	switch (wParam)
	{
	case 'W': // Move forward
		m_moveForward = false;
		break;
	case 'S': // Move backward
		m_moveBackward = false;
		break;
	case 'A': // Move left (strafe)
		m_moveLeft = false;
		break;
	case 'D': // Move right (strafe)
		m_moveRight = false;

		break;
	case VK_SPACE: // Move up
		m_moveUp = false;
		break;
	case VK_SHIFT: // Move up
		m_moveDown = false;
		break;
	}


}

void FPSCamera::OnMouseMove(int deltaX, int deltaY)
{


	// Update the camera's rotation based on the mouse movement
	m_rotation.x -= static_cast<float>(deltaY) * m_deltaTime * 3.0f; // Adjust the sensitivity as needed
	m_rotation.y -= static_cast<float>(deltaX) * m_deltaTime * 3.0f;

	// Limit the pitch to avoid gimbal lock (optional)
	const float maxPitch = 90.f - 0.01f;
	m_rotation.x = max(-maxPitch, min(maxPitch, m_rotation.x));

	// Update the view matrix with the new rotation
	UpdateViewMatrix();


	
	

}


// Set camera position
void FPSCamera::SetPosition(const DirectX::XMFLOAT3& position)
{
	m_position = position;
	UpdateViewMatrix(); // Update the view matrix to reflect the new position
}

// Set camera rotation
void FPSCamera::SetRotation(float pitch, float yaw)
{
	m_rotation.x = pitch;
	m_rotation.y = yaw;
	UpdateViewMatrix(); // Update the view matrix to reflect the new rotation
}

// Get the view matrix
DirectX::XMMATRIX FPSCamera::GetViewMatrix() const
{
	return m_viewMatrix;
}

// Get the projection matrix
DirectX::XMMATRIX FPSCamera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

void FPSCamera::SetDeltaTime(float deltaTime)
{
	m_deltaTime = deltaTime;
}

void FPSCamera::Update(float* frameIndex, bool* cameraMoved)
{
	using namespace DirectX;
	float speed = m_deltaTime * 10.0f;

	if (m_moveForward)
	{
		m_position.z -= speed * m_forward.z;
		m_position.x -= speed * m_forward.x;
	}
	if (m_moveBackward)
	{
		m_position.z += speed * m_forward.z;
		m_position.x += speed * m_forward.x;
	}
	if (m_moveLeft)
	{
		m_position.x += speed * m_right.x;
		m_position.z += speed * m_right.z;
	}
	if (m_moveRight)
	{
		m_position.x -= speed * m_right.x;
		m_position.z -= speed * m_right.z;
	}
	if (m_moveUp)
	{
		m_position.y += speed;
	}
	if (m_moveDown)
	{
		m_position.y -= speed;
	}

	bool moved = m_moveForward || m_moveBackward || m_moveLeft || m_moveRight || m_moveUp || m_moveDown;
	if (moved)
	{
		*cameraMoved = true;
		*frameIndex = 0;
	}

	// Update the view matrix after changing the camera's position
	UpdateViewMatrix();
}