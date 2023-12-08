#include "stdafx.h"
#include "BLASBuilder.h"
#include <stdexcept>

//#include "DXRHelper.h"
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

#ifndef ROUND_UP
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#endif

// Specifies a heap used for uploading. This heap type has CPU access optimized
// for uploading to the GPU.
static const D3D12_HEAP_PROPERTIES kUploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

// Specifies the default heap. This heap type experiences the most bandwidth for
// the GPU, but cannot provide CPU access.
static const D3D12_HEAP_PROPERTIES kDefaultHeapProps = {
	D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

BLASBuilder::BLASBuilder(ComPtr<ID3D12Device5> device) : m_device(device) 
{
}

void BLASBuilder::AddGeometry(BaseObjectClass* object) {
	// Maybe check if the geometry is visible???
	m_geometry.push_back(object);
	D3D12_RAYTRACING_GEOMETRY_DESC gDesc = object->getGeometryDescription();
	if (gDesc.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS)
	{

	}

	m_geometryCount++;
}

std::vector<ComPtr<ID3D12Resource>> BLASBuilder::BuildBLAS(ComPtr<ID3D12GraphicsCommandList4> commandList)
{
	std::vector<ComPtr<ID3D12Resource>> buffers;
	for (UINT i = 0; i < m_geometryCount; ++i)
	{

		// The AS build requires some scratch space to store temporary information.
		// The amount of scratch memory is dependent on the scene complexity.
		// The final AS also needs to be stored in addition to the existing vertex
		// buffers. It size is also dependent on the scene complexity.
		ComputeASBufferSizes(false, &m_geometry[i]->asBuffers, m_geometry[i]->getGeometryDescription());

		// Once the sizes are obtained, the application is responsible for allocating
		// the necessary buffers. Since the entire generation will be done on the GPU,
		// we can directly allocate those on the default heap
		m_geometry[i]->asBuffers.pScratch = CreateBuffer(
			m_device.Get(), m_geometry[i]->asBuffers.scratchSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
			kDefaultHeapProps);
		m_geometry[i]->asBuffers.pResult = CreateBuffer(
			m_device.Get(), m_geometry[i]->asBuffers.resultSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			kDefaultHeapProps);

		m_geometry[i]->buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

		// Build the acceleration structure. Note that this call integrates a barrier
		// on the generated AS, so that it can be used to compute a top-level AS right
		// after this method.
		Generate(commandList.Get(), &m_geometry[i]->asBuffers, m_geometry[i]->getGeometryDescription(), m_geometry[i]->buildFlags, false, nullptr);

		buffers.push_back(m_geometry[i]->asBuffers.pResult);
	}
	
	return buffers;

}


//--------------------------------------------------------------------------------------------------
// Compute the size of the scratch space required to build the acceleration
// structure, as well as the size of the resulting structure. The allocation of
// the buffers is then left to the application
void BLASBuilder::ComputeASBufferSizes(
	bool allowUpdate,     // If true, the resulting acceleration structure will
						  // allow iterative updates
	AccelerationStructureBuffers* asBuffers,	// Will set the required scratch memory on the GPU to build (pScratch)
												// the acceleration structure and the required GPU memory to store the acceleration
												// structure (pResult)
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescription
) {
	

	std::vector< D3D12_RAYTRACING_GEOMETRY_DESC> descs{ geometryDescription };
	// Describe the work being requested, in this case the construction of a
	// (possibly dynamic) bottom-level hierarchy, with the given vertex buffers

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc;
	prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	prebuildDesc.NumDescs = descs.size(); //////////////////
	prebuildDesc.pGeometryDescs = descs.data();
	prebuildDesc.Flags = allowUpdate
		? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
		: D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	// This structure is used to hold the sizes of the required scratch memory and
	// resulting AS
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

	// Building the acceleration structure (AS) requires some scratch space, as
	// well as space to store the resulting structure This function computes a
	// conservative estimate of the memory requirements for both, based on the
	// geometry size.
	m_device->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

	// Buffer sizes need to be 256-byte-aligned
	asBuffers->scratchSizeInBytes =
		ROUND_UP(info.ScratchDataSizeInBytes,
			D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	asBuffers->resultSizeInBytes = ROUND_UP(info.ResultDataMaxSizeInBytes,
		D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}


//--------------------------------------------------------------------------------------------------
// Enqueue the construction of the acceleration structure on a command list,
// using application-provided buffers and possibly a pointer to the previous
// acceleration structure in case of iterative updates. Note that the update can
// be done in place: the result and previousResult pointers can be the same.
void BLASBuilder::Generate(
	ID3D12GraphicsCommandList4* commandList, // Command list on which the build will be enqueued
	AccelerationStructureBuffers* asBuffers, 
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescription,
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
	bool updateOnly,   // If true, simply refit the existing
					   // acceleration structure
	ID3D12Resource* previousResult // Optional previous acceleration
								   // structure, used if an iterative update
								   // is requested
) {

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = buildFlags;
	// The stored flags represent whether the AS has been built for updates or
	// not. If yes and an update is requested, the builder is told to only update
	// the AS instead of fully rebuilding it
	if (flags ==
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE &&
		updateOnly) {
		flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	}

	// Sanity checks
	if (buildFlags !=
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE &&
		updateOnly) {
		throw std::logic_error(
			"Cannot update a bottom-level AS not originally built for updates");
	}
	if (updateOnly && previousResult == nullptr) {
		throw std::logic_error(
			"Bottom-level hierarchy update requires the previous hierarchy");
	}

	if (asBuffers->resultSizeInBytes == 0 || asBuffers->scratchSizeInBytes == 0) {
		throw std::logic_error(
			"Invalid scratch and result buffer sizes - ComputeASBufferSizes needs "
			"to be called before Build");
	}

	std::vector< D3D12_RAYTRACING_GEOMETRY_DESC> descs{ geometryDescription };


	// Create a descriptor of the requested builder work, to generate a
	// bottom-level AS from the input parameters
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
	buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	buildDesc.Inputs.NumDescs = descs.size();
	buildDesc.Inputs.pGeometryDescs = descs.data();
	buildDesc.DestAccelerationStructureData = {
		asBuffers->pResult.Get()->GetGPUVirtualAddress() };
	buildDesc.ScratchAccelerationStructureData = {
		asBuffers->pScratch.Get()->GetGPUVirtualAddress() };
	buildDesc.SourceAccelerationStructureData =
		previousResult ? previousResult->GetGPUVirtualAddress() : 0;
	buildDesc.Inputs.Flags = flags;

	// Build the AS
	commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// Wait for the builder to complete by setting a barrier on the resulting
	// buffer. This is particularly important as the construction of the top-level
	// hierarchy may be called right afterwards, before executing the command
	// list.
	D3D12_RESOURCE_BARRIER uavBarrier;
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = asBuffers->pResult.Get();
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandList->ResourceBarrier(1, &uavBarrier);
}