#include "stdafx.h"
#include "BaseObjectClass.h"
#include <stdexcept>

Microsoft::WRL::ComPtr<ID3D12Resource> BaseObjectClass::m_materialBuffer;
std::vector<Material> BaseObjectClass::m_materials;

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}


void BaseObjectClass::addInstance(XMMATRIX transformation, Material material, std::wstring hitGroup, std::wstring shadowHitGroup)
{
	Instance instance;
	instance.transform = transformation;
	instance.invTransform = XMMatrixInverse(nullptr, transformation);

	m_instances.push_back(instance);
	m_materials.push_back(material);
	m_hitGroups.push_back({ hitGroup, shadowHitGroup });

	addDerivedInstance();

	totalInstanceCount++;
}

void BaseObjectClass::transformInstance(XMMATRIX transformation, UINT index)
{	
	Instance& instance = m_instances[index];

	instance.transform = transformation * instance.transform;
	instance.invTransform = XMMatrixInverse(nullptr, instance.transform);
}

void BaseObjectClass::setTransformInstance(XMMATRIX transformation, UINT index)
{
	Instance& instance = m_instances[index];

	instance.transform = transformation;
	instance.invTransform = XMMatrixInverse(nullptr, instance.transform);
}

void BaseObjectClass::translateInstance(float OffsetX, float OffsetY, float OffsetZ, UINT index)
{
	Instance& instance = m_instances[index];

	instance.transform = XMMatrixTranslation(OffsetX, OffsetY, OffsetZ) * instance.transform;
	instance.invTransform = XMMatrixInverse(nullptr, instance.transform);
}

void BaseObjectClass::rotateInstance(FXMVECTOR axis, float angle, UINT index)
{
	Instance& instance = m_instances[index];

	instance.transform = XMMatrixRotationAxis(axis, angle) * instance.transform;
	instance.invTransform = XMMatrixInverse(nullptr, instance.transform);
}

void BaseObjectClass::scaleInstance(float ScaleX, float ScaleY, float ScaleZ, UINT index)
{
	Instance& instance = m_instances[index];

	instance.transform = XMMatrixScaling(ScaleX, ScaleY, ScaleZ) * instance.transform;
	instance.invTransform = XMMatrixInverse(nullptr, instance.transform);
}

ComPtr<ID3D12RootSignature> BaseObjectClass::CreateHitGroupSignature()
{
	RootSignatureBuilder rsc;

	//rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV,
	   // 0 /*t0*/); // spheres

	// Additional binding for the camera buffer, using a different register, like b1
	rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, 0 /*b0*/); // transformation

	// #DXR Extra - Another ray type
	// Add a single range pointing to the TLAS in the heap
	rsc.AddHeapRangesParameter({
		{0 /*t0*/, 1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		 1 /*2nd slot of the heap*/},
		 {0 /*b0*/, 1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_CBV /*Camera parameters*/,
		  2} ,
		 {1 /*b1*/, 1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_CBV /*scene parameters*/,
		  3}
		});
	return rsc.Generate(m_device.Get(), true);
}

void BaseObjectClass::createMaterialBuffers()
{
	// Define the number of elements in the buffer
	const UINT elementCount = totalInstanceCount; // Define the number of materials

	// Create the buffer
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeof(Material) * elementCount; // Total size of the buffer
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	m_materialBuffer = nullptr;
	HRESULT hr = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,	
		nullptr,
		IID_PPV_ARGS(&m_materialBuffer));

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create material buffer.");
	}
}


void BaseObjectClass::updateMaterialBuffers() {
	uint8_t* pData = nullptr;
	ThrowIfFailed(m_materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

	if (pData) {
		size_t dataSize = sizeof(Material) * totalInstanceCount;
		std::memcpy(pData, m_materials.data(), dataSize);
		m_materialBuffer->Unmap(0, nullptr); // Unmap the buffer
	}
	else {
		throw std::runtime_error("Unable to map material buffer.");
	}
}


ComPtr<ID3D12Resource> BaseObjectClass::getMaterialBuffer()
{
	return m_materialBuffer;
}