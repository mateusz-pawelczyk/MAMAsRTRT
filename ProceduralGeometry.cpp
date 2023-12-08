#include "stdafx.h"
#include "ProceduralGeometry.h"

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

inline void AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
	void* pMappedData;
	(*ppResource)->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	(*ppResource)->Unmap(0, nullptr);
}

ProceduralGeometry::ProceduralGeometry(DirectX::XMFLOAT3 aabbMin, DirectX::XMFLOAT3 aabbMax, ComPtr<ID3D12Device5> device)
{
	m_aabb = { aabbMin.x, aabbMin.y, aabbMin.z, aabbMax.x, aabbMax.y, aabbMax.z };
	AllocateUploadBuffer(device.Get(), &m_aabb, sizeof(m_aabb), &m_aabbBuffer);

	setGeometryDescription();


}

ProceduralGeometry::ProceduralGeometry(float aabbMinX, float aabbMinY, float aabbMinZ, float aabbMaxX, float aabbMaxY, float aabbMaxZ, ComPtr<ID3D12Device5> device)
{
	m_aabb = { aabbMinX, aabbMinY, aabbMinZ, aabbMaxX, aabbMaxY, aabbMaxZ };
	AllocateUploadBuffer(device.Get(), &m_aabb, sizeof(m_aabb), &m_aabbBuffer);

	setGeometryDescription();
}

void ProceduralGeometry::setGeometryDescription()
{
	m_geometryDescription.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
	m_geometryDescription.AABBs.AABBCount = 1;
	m_geometryDescription.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
	m_geometryDescription.AABBs.AABBs.StartAddress = m_aabbBuffer->GetGPUVirtualAddress();
	m_geometryDescription.Flags = m_visibility ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
}

void ProceduralGeometry::setGeometryDescription(D3D12_RAYTRACING_GEOMETRY_DESC newGeometryDescription)
{
	m_geometryDescription = newGeometryDescription;
}