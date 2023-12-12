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

