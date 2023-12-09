#pragma once
#include "ProceduralGeometry.h"
class Sphere :
	public ProceduralGeometry
{
private:
	float m_radius;
	DirectX::XMFLOAT3 m_center;

public:
	Sphere(float radius, ComPtr<ID3D12Device5> device);
};

