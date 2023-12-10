#pragma once


#include "BaseObjectClass.h"
#include <vector>

// ProceduralGeometry class inherits from BaseObjectClass
class ProceduralGeometry : public BaseObjectClass {
protected:
	D3D12_RAYTRACING_AABB m_aabb;
	ComPtr<ID3D12Resource> m_aabbBuffer;
	void setGeometryDescription() override;
	void setGeometryDescription(D3D12_RAYTRACING_GEOMETRY_DESC newGeometryDescription);

	void createAABB(float aabbMinX, float aabbMinY, float aabbMinZ, float aabbMaxX, float aabbMaxY, float aabbMaxZ);
	void createAABB(DirectX::XMFLOAT3 aabbMin, DirectX::XMFLOAT3 aabbMax);
	ComPtr<ID3D12Resource> m_blasResource;
	std::vector<ComPtr<ID3D12Resource>> m_instanceBuffers;

public:
	void createInstanceBuffers();
	void updateInstanceBuffers();

	ComPtr<ID3D12Resource> getInstanceBuffer(UINT index) { return m_instanceBuffers[index]; }
	ComPtr<ID3D12RootSignature> CreateHitGroupSignature() override;
};

