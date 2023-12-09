#include "Common.hlsl"



struct Sphere {
	float3 center;
	float radius;
};
cbuffer SceneConstantBuffer : register(b1, space1) {
	float4 lightPosition;
	float4 lightAmbientColor;
	float4 lightDiffuseColor;
	float  elapsedTime;
}

cbuffer Transform : register(b0) {
	float4x4 transform;
	float4x4 invTransform;
}
//StructuredBuffer<Sphere> g_Spheres : register(t0);

float3 objectRayOrigin()
{
	return mul(invTransform, float4(WorldRayOrigin(), 1.0f)).xyz;
}

float3 objectRaDirection()
{
	return mul(invTransform, float4(WorldRayDirection(), 0.0f)).xyz;
}


[shader("intersection")] void SphereIntersection()
{
	//Attributes attr;
	//attr.bary = float2(0.0f, 0.0f);
	//ReportHit(4.0f, 0, attr);

	// Retrieve instance index
	uint instanceIndex = PrimitiveIndex();

	// Access sphere data using the instance index
	Sphere sphere;// = g_Spheres[instanceIndex];
	sphere.center = float3(-0.5f, -0.5f, -0.5f);
	sphere.radius = 0.4f;

	SphereAttributes attr;

	float3 oc = objectRayOrigin();
	float a = dot(objectRaDirection(), objectRaDirection());
	float half_b = dot(oc, objectRaDirection());
	float c = dot(oc, oc) - sphere.radius * sphere.radius;
	float discriminant = half_b * half_b - a * c;

	
	float t = (-half_b - sqrt(discriminant)) / a;

	
	if (t >= 0)
	{
		float3 P = objectRayOrigin() + t * objectRaDirection();

		//float3 N = normalize(WorldRayOrigin() + t * WorldRayDirection() - sphere.center);

		//// Calculate UV coordinates
		float U = 0.5 + atan2(P.z, P.x) / (2.0 * 3.14159265359);
		float V = 0.5 - asin(P.y) / 3.14159265359;
		//
		////attr.normal = N;
		attr.uv = float2(U, V);

		ReportHit(t, 0, attr);
	}
	
	
}