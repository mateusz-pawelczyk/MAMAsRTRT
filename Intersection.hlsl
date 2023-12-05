#include "Common.hlsl"



struct Sphere {
	float3 center;
	float radius;
};

//StructuredBuffer<Sphere> g_Spheres : register(t0);

[shader("intersection")]
void SphereIntersection()
{
	Attributes attr;
	attr.bary = float2(0.0f, 0.0f);
	ReportHit(0.0f, 0, attr);

	//// Retrieve instance index
	//uint instanceIndex = PrimitiveIndex();

	//// Access sphere data using the instance index
	//Sphere sphere;// = g_Spheres[instanceIndex];
	//sphere.center = float3(1.0f, 1.0f, 1.0f);
	//sphere.radius = 1.0f;

	//SphereAttributes attr;

	//float3 oc = WorldRayOrigin() - sphere.center;
	//float a = dot(WorldRayDirection(), WorldRayDirection());
	//float half_b = dot(oc, WorldRayDirection());
	//float c = dot(oc, oc) - sphere.radius * sphere.radius;
	//float discriminant = half_b * half_b - a * c;

	//
	//float t = (-half_b - sqrt(discriminant)) / a;


	//
	//if (t >= 0)
	//{
	//	//float3 N = normalize(WorldRayOrigin() + t * WorldRayDirection() - sphere.center);

	//	//// Calculate UV coordinates
	//	//float U = 0.5 + atan2(P.z, P.x) / (2.0 * 3.14159265359);
	//	//float V = 0.5 - asin(P.y) / 3.14159265359;
	//	//
	//	////attr.normal = N;
	//	//attr.uv = float2(U, V);

	//	ReportHit(t, 0, attr);
	//}
	
	
}