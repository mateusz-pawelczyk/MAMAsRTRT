#include "stdafx.h"
#include "ProceduralGeometry.h"
#include <stdexcept>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

inline ID3D12Resource* CreateBuffer(ID3D12Device* m_device, uint64_t size,
	D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState,
	const D3D12_HEAP_PROPERTIES& heapProps)
{
	D3D12_RESOURCE_DESC bufDesc = {};
	bufDesc.Alignment = 0;
	bufDesc.DepthOrArraySize = 1;
	bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufDesc.Flags = flags;
	bufDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufDesc.Height = 1;
	bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufDesc.MipLevels = 1;
	bufDesc.SampleDesc.Count = 1;
	bufDesc.SampleDesc.Quality = 0;
	bufDesc.Width = size;

	ID3D12Resource* pBuffer;
	ThrowIfFailed(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
		initState, nullptr, IID_PPV_ARGS(&pBuffer)));
	return pBuffer;
}

// Specifies a heap used for uploading. This heap type has CPU access optimized
// for uploading to the GPU.
static const D3D12_HEAP_PROPERTIES kUploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

// Specifies the default heap. This heap type experiences the most bandwidth for
// the GPU, but cannot provide CPU access.
static const D3D12_HEAP_PROPERTIES kDefaultHeapProps = {
	D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

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


void ProceduralGeometry::createAABB(float aabbMinX, float aabbMinY, float aabbMinZ, float aabbMaxX, float aabbMaxY, float aabbMaxZ)
{
	m_aabb = { aabbMinX, aabbMinY, aabbMinZ, aabbMaxX, aabbMaxY, aabbMaxZ };
	AllocateUploadBuffer(m_device.Get(), &m_aabb, sizeof(m_aabb), &m_aabbBuffer);
}

void ProceduralGeometry::createAABB(DirectX::XMFLOAT3 aabbMin, DirectX::XMFLOAT3 aabbMax)
{
	m_aabb = { aabbMin.x, aabbMin.y, aabbMin.z, aabbMax.x, aabbMax.y, aabbMax.z };
	AllocateUploadBuffer(m_device.Get(), &m_aabb, sizeof(m_aabb), &m_aabbBuffer);
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

void ProceduralGeometry::createInstanceBuffers()
{
	uint32_t bufferSize = (sizeof(Instance) + 255) & ~255; // Align to 256 bytes

	for (int i = 0; i < m_instances.size(); ++i)
	{
		m_instanceBuffers.push_back(CreateBuffer(
			m_device.Get(), bufferSize, D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps));
	}
	
}

void ProceduralGeometry::updateInstanceBuffers()
{
	for (int i = 0; i < m_instances.size(); ++i)
	{
		uint8_t* pData = nullptr;
		ThrowIfFailed(m_instanceBuffers[i]->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

		if (pData) {
			memcpy(pData, &m_instances[i], sizeof(Instance)); // Copy actual data size
			m_instanceBuffers[i]->Unmap(0, nullptr);
		}
		else {
			throw std::runtime_error("Unable to map instance constant buffer.");
		}
	}
}