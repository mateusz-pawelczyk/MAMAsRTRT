#pragma once


#include <DirectXMath.h>
#include "ObjectStructs.h"
#include "RootSignatureBuilder.h"
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
struct Instance
{
	XMMATRIX transform;
	XMMATRIX invTransform;
};

struct alignas(16)  Material {
	XMVECTOR diffuseColor;
	XMVECTOR specularColor;
	XMVECTOR emissiveColor;
	float specularPower;
	float emissiveness;
	float reflectivity;
	float refractivity;
	float refractionIndex;

};

class BaseObjectClass {
protected:
	static std::vector<Material> m_materials;

	bool m_visibility;
	ComPtr<ID3D12Device5> m_device;
	D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDescription = {};
	virtual void setGeometryDescription() {};
	std::vector<Instance> m_instances;
	

public:
	static UINT totalInstanceCount;
	static ComPtr<ID3D12Resource> m_materialBuffer;

	D3D12_RAYTRACING_GEOMETRY_DESC getGeometryDescription() { return m_geometryDescription; };
	AccelerationStructureBuffers asBuffers;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags;

	void addInstance(Material material);
	void addInstance(XMMATRIX transformation, Material material);
	void transformInstance(XMMATRIX transformation, UINT index);
	void setTransformInstance(XMMATRIX transformation, UINT index);
	void translateInstance(float OffsetX, float OffsetY, float OffsetZ, UINT index);
	void rotateInstance(FXMVECTOR axis, float angle, UINT index);
	void scaleInstance(float ScaleX, float ScaleY, float ScaleZ, UINT index);
	Instance getInstance(UINT index) { return m_instances[index]; }
	UINT getInstanceCount() { return m_instances.size();  }

	virtual ComPtr<ID3D12RootSignature> CreateHitGroupSignature();
	void createMaterialBuffers();
	void updateMaterialBuffers();

	ComPtr<ID3D12Resource> getMaterialBuffer();
};


