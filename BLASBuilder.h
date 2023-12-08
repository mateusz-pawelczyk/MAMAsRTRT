#pragma once

#include <vector>
#include "BaseObjectClass.h"

#include "ObjectStructs.h"

class BLASBuilder {
private:
	ComPtr<ID3D12Device5> m_device;
	std::vector<BaseObjectClass*> m_geometry;
	UINT m_geometryCount = 0;

	// -------
	// Methods 

	void ComputeASBufferSizes(bool allowUpdate, AccelerationStructureBuffers* asBuffers, D3D12_RAYTRACING_GEOMETRY_DESC geometryDescription);

	void Generate(
		ID3D12GraphicsCommandList4* commandList,
		AccelerationStructureBuffers* asBuffers,
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDescription,
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
		bool updateOnly,
		ID3D12Resource* previousResult
	);
public:
	BLASBuilder(ComPtr<ID3D12Device5> device);

	void AddGeometry(BaseObjectClass* object);

	std::vector<ComPtr<ID3D12Resource>> BuildBLAS(ComPtr<ID3D12GraphicsCommandList4> commandList);

	
};
