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

float3 objectRayDirection()
{
	return mul(invTransform, float4(WorldRayDirection(), 0.0f)).xyz;
}


[shader("intersection")] void SphereIntersection()
{
	SphereAttributes attr;

	uint instanceIndex = PrimitiveIndex();
	Sphere sphere;
	sphere.center = float3(-0.5f, -0.5f, -0.5f);
	sphere.radius = 0.4f;

	float3 oc = objectRayOrigin();
	float a = dot(objectRayDirection(), objectRayDirection());
	float half_b = dot(oc, objectRayDirection());
	float c = dot(oc, oc) - sphere.radius * sphere.radius;
	float discriminant = half_b * half_b - a * c;

	if (discriminant > 0) {
		float sqrtd = sqrt(discriminant);
		float t = (-half_b - sqrtd) / a; // Only considering the nearest intersection
		float t2 = (-half_b + sqrtd) / a; // Only considering the nearest intersection

		if (t >= RayTMin()) {
			float3 P = objectRayOrigin() + t * objectRayDirection();
			float3 N = normalize(P - sphere.center); // Normal at the intersection

			// Calculate UV coordinates
			float u = atan2(N.z, N.x) / (2 * 3.14159265359f);
			float v = (acos(N.y / sphere.radius) / 3.14159265359f);
			attr.uv = float2(u, 1.0f - v); // Adjust V if necessary
			attr.normal = normalize(P);
			ReportHit(t, 0, attr);
		}
		//else if (t2 > RayTMin()) // if 
		//{
		//	float3 P = objectRayOrigin() + t2 * objectRayDirection();
		//	float3 N = normalize(P - sphere.center); // Normal at the intersection

		//	// Calculate UV coordinates
		//	float u = atan2(N.z, N.x) / (2 * 3.14159265359f);
		//	float v = (acos(N.y / sphere.radius) / 3.14159265359f);
		//	attr.uv = float2(u, 1.0f - v); // Adjust V if necessary
		//	attr.normal = P;
		//	ReportHit(t2, 0, attr);
		//}
	}
	
	
}

