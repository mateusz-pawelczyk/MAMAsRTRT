#pragma once


#include "BaseObjectClass.h"

// ProceduralGeometry class inherits from BaseObjectClass
class ProceduralGeometry : public BaseObjectClass {
private:
	D3D12_RAYTRACING_AABB m_aabb;
	ComPtr<ID3D12Resource> m_aabbBuffer;
	void setGeometryDescription() override;

public:
	ProceduralGeometry(DirectX::XMFLOAT3 aabbMin, DirectX::XMFLOAT3 aabbMax, ComPtr<ID3D12Device5> device);
	ProceduralGeometry(float aabbMinX, float aabbMinY, float aabbMinZ, float aabbMaxX, float aabbMaxY, float aabbMaxZ, ComPtr<ID3D12Device5> device);
	void setGeometryDescription(D3D12_RAYTRACING_GEOMETRY_DESC newGeometryDescription);

};

