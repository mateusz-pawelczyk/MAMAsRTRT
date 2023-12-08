#pragma once


#include <DirectXMath.h>
#include "ObjectStructs.h"

using Microsoft::WRL::ComPtr;



class BaseObjectClass {
protected:
	DirectX::XMFLOAT3 m_transformPosition;
	float m_transformRotation;
	float m_transformScale;
	//Material* material; // Assume Material is a defined class
	bool m_visibility;
	ComPtr<ID3D12Device5> m_device;
	D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDescription = {};
	virtual void setGeometryDescription() {};

public:
	BaseObjectClass() : m_transformPosition(0, 0, 0), m_transformRotation(0), m_transformScale(1),/* material(nullptr),*/ m_visibility(true){}
	D3D12_RAYTRACING_GEOMETRY_DESC getGeometryDescription() { return m_geometryDescription; };
	AccelerationStructureBuffers asBuffers;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags;
};


