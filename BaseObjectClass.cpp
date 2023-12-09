#include "stdafx.h"
#include "BaseObjectClass.h"

void BaseObjectClass::addInstance()
{
	Instance instance;
	instance.transform = XMMatrixIdentity();
	instance.invTransform = XMMatrixIdentity();

	m_instances.push_back(instance);
}

void BaseObjectClass::addInstance(XMMATRIX transformation)
{
	Instance instance;
	instance.transform = transformation;
	instance.invTransform = XMMatrixInverse(nullptr, transformation);

	m_instances.push_back(instance);
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
