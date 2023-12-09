#pragma once


#include <DirectXMath.h>
#include "ObjectStructs.h"
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
struct Instance
{
	XMMATRIX transform;
	XMMATRIX invTransform;
};


class BaseObjectClass {
protected:

	//Material* material; // Assume Material is a defined class
	bool m_visibility;
	ComPtr<ID3D12Device5> m_device;
	D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDescription = {};
	virtual void setGeometryDescription() {};
	std::vector<Instance> m_instances;

public:
	D3D12_RAYTRACING_GEOMETRY_DESC getGeometryDescription() { return m_geometryDescription; };
	AccelerationStructureBuffers asBuffers;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags;

	void addInstance();
	void addInstance(XMMATRIX transformation);
	void transformInstance(XMMATRIX transformation, UINT index);
	void setTransformInstance(XMMATRIX transformation, UINT index);
	void translateInstance(float OffsetX, float OffsetY, float OffsetZ, UINT index);
	void rotateInstance(FXMVECTOR axis, float angle, UINT index);
	void scaleInstance(float ScaleX, float ScaleY, float ScaleZ, UINT index);
	Instance getInstance(UINT index) { return m_instances[index]; }
	UINT getInstanceCount() { return m_instances.size();  }
};


