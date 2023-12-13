#pragma once


#include <DirectXMath.h>
#include "ObjectStructs.h"
#include "RootSignatureBuilder.h"
#include <variant>
#include <variant>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
struct Instance
{
	XMMATRIX transform;
	XMMATRIX invTransform;

};

struct SphereInfo
{
	float radius;
};


// Make sure to use correct alignment! In this case (maybe also in general) we had to make sure that the accumulated sizes add up to a multiple of 16.
// Otherwise the access in the shader will not be correct.
//struct alignas(16)  Material {
//	XMVECTOR diffuseColor;
//	XMVECTOR specularColor;
//	XMVECTOR emissiveColor;
//	float emissiveness;		// how diffuse
//	float reflectivity;		
//	float refractivity;
//	float refractionIndex;
//	float fuzz;				// metalness
//	float matte;			// how matte 
//	float padding[2];		// <---- adding padding for alignment
//
//};




struct alignas(16)  Material {
	XMVECTOR attenuation;
	float refractionIndex;
	float fuzz;				// metalness
	float matte;			// how matte 
	float padding;			// <---- adding padding for alignment
};

class BaseObjectClass {
protected:
	static std::vector<Material> m_materials;

	bool m_visibility;
	ComPtr<ID3D12Device5> m_device;
	D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDescription = {};
	virtual void setGeometryDescription() {};
	std::vector<Instance> m_instances;
	virtual void addDerivedInstance() {};
	
public:
	static UINT totalInstanceCount;
	static ComPtr<ID3D12Resource> m_materialBuffer;
	static std::vector<std::pair<std::wstring, std::wstring>> m_hitGroups;

	using GeometryVariant = std::variant<SphereInfo>;
	static std::vector<GeometryVariant> geometries;
	//static std::pair<const std::wstring&, const std::wstring&> getHitGroupPair(UINT index) { return m_hitGroups[index]; }

	D3D12_RAYTRACING_GEOMETRY_DESC getGeometryDescription() { return m_geometryDescription; };
	AccelerationStructureBuffers asBuffers;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags;

	virtual void addInstance(Material material, std::wstring hitGroup, std::wstring shadowHitGroup);
	virtual void addInstance(XMMATRIX transformation, Material material, std::wstring hitGroup, std::wstring shadowHitGroup);
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


