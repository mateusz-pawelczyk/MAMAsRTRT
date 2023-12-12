#pragma once
#include "Mesh.h"
#include "stdafx.h"
class Plane :
	public Mesh
{
private:
	static UINT planeInstanceCount;
	DirectX::XMFLOAT3 m_planeMin;
	DirectX::XMFLOAT3 m_planeMax;

	void Plane::GenerateVertices();
	void Plane::GenerateIndices();
	
public:
	Plane(XMFLOAT3 planeMin, XMFLOAT3 planeMax, ComPtr<ID3D12Device5> device);
	void addDerivedInstance() override;
	UINT getPlaneInstanceCount() { return planeInstanceCount; }

};

