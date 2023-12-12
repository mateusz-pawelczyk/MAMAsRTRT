#include "stdafx.h"
#include "Plane.h"

UINT Plane::planeInstanceCount = 0;
void Plane::addDerivedInstance()
{
	planeInstanceCount++; 
}


Plane::Plane(XMFLOAT3 planeMin, XMFLOAT3 planeMax, ComPtr<ID3D12Device5> device) : Mesh(device)
{
	m_planeMin = planeMin;
	m_planeMax = planeMax;

	GenerateVertices();
	GenerateIndices();

	setGeometryDescription();
}

void Plane::GenerateVertices()
{
	std::vector<Vertex> vertices;

	// Corners
	DirectX::XMFLOAT3 bottomLeft(m_planeMin.x, m_planeMin.y, m_planeMin.z);
	DirectX::XMFLOAT3 topLeft(m_planeMin.x, m_planeMin.y, m_planeMax.z);
	DirectX::XMFLOAT3 bottomRight(m_planeMax.x, m_planeMax.y, m_planeMin.z);
	DirectX::XMFLOAT3 topRight(m_planeMax.x, m_planeMax.y, m_planeMax.z);

	// Calculate normal using cross product (assuming counter-clockwise winding order)
	DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&topLeft) - DirectX::XMLoadFloat3(&bottomLeft);
	DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&bottomRight) - DirectX::XMLoadFloat3(&bottomLeft);
	DirectX::XMVECTOR normalVec = DirectX::XMVector3Cross(v1, v2);
	DirectX::XMFLOAT3 normal;
	DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(normalVec));

	// Default color (change as needed)
	DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f); // White color

	// Texture coordinates (assuming standard mapping)
	DirectX::XMFLOAT2 texBL(0.0f, 1.0f); // Bottom left
	DirectX::XMFLOAT2 texTL(0.0f, 0.0f); // Top left
	DirectX::XMFLOAT2 texBR(1.0f, 1.0f); // Bottom right
	DirectX::XMFLOAT2 texTR(1.0f, 0.0f); // Top right

	// Tangents and binormals (can be calculated based on your specific needs)
	DirectX::XMFLOAT3 tangent(1.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 binormal(0.0f, 0.0f, 1.0f);

	// Create vertices
	vertices.push_back(Vertex(bottomLeft, color, normal, texBL, tangent, binormal));
	vertices.push_back(Vertex(topLeft, color, normal, texTL, tangent, binormal));
	vertices.push_back(Vertex(bottomRight, color, normal, texBR, tangent, binormal));
	vertices.push_back(Vertex(topRight, color, normal, texTR, tangent, binormal));

	createVertexBuffer(vertices);
}

void Plane::GenerateIndices()
{
	std::vector<UINT> indices;

	// First triangle (bottom left, top left, top right)
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);

	// Second triangle (bottom left, top right, bottom right)
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(2);

	createIndexBuffer(indices);
}