#include "stdafx.h"
#include "Sphere.h"

UINT Sphere::sphereInstanceCount = 0;
void Sphere::addDerivedInstance()  { sphereInstanceCount++; }


Sphere::Sphere(float radius, ComPtr<ID3D12Device5> device)
{
	m_device = device;

	createAABB(-radius, -radius, -radius, radius, radius, radius);

	setGeometryDescription();
}



void Sphere::addSphereInstance(XMMATRIX transformation, Material material, std::wstring hitGroup, std::wstring shadowHitGroup, float radius)
{
	BaseObjectClass::addInstance(XMMatrixScaling(radius, radius, radius) *  transformation, material, hitGroup,shadowHitGroup);
	
	SphereInfo info;
	info.radius = radius;
	
	m_sphereInfos.push_back(info);
}