#pragma once
#include "ProceduralGeometry.h"
class Sphere :
	public ProceduralGeometry
{
protected:
	float m_radius;
	DirectX::XMFLOAT3 m_center;
	static UINT sphereInstanceCount;
	
public:
	Sphere(float radius, ComPtr<ID3D12Device5> device);
	UINT getSphereInstanceCount() { return sphereInstanceCount; }
	void addDerivedInstance() override;
};

