#pragma once
#include "ProceduralGeometry.h"

struct SphereInfo
{
	float radius;
};

class Sphere :
	public ProceduralGeometry
{
protected:
	float m_radius;
	DirectX::XMFLOAT3 m_center;
	static UINT sphereInstanceCount;
	std::vector<SphereInfo> m_sphereInfos;
	
public:
	Sphere(float radius, ComPtr<ID3D12Device5> device);
	UINT getSphereInstanceCount() { return sphereInstanceCount; }
	void addDerivedInstance() override;
	void addSphereInstance(XMMATRIX transformation, Material material, std::wstring hitGroup, std::wstring shadowHitGroup, float radius);

	SphereInfo getSphereInfoForInstance(UINT index) { return m_sphereInfos[index];  }
};

