#include "Common.hlsl"

// #DXR Extra - Another ray type
// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0, space1);

cbuffer CameraParams : register(b0, space1)
{
	float4x4 view;
	float4x4 projection;
	float4x4 viewI;
	float4x4 projectionI;
}
struct ShadowHitInfo {
	bool isHit;
};

cbuffer SceneConstantBuffer : register(b1, space1) {
	float4 lightPosition;
	float4 lightAmbientColor;
	float4 lightDiffuseColor;
	float  elapsedTime;
}

struct Material {
	float4 diffuseColor;
	float4 specularColor;
	float4 emissiveColor;
	float emissiveness;		// how diffuse
	float reflectivity;
	float refractivity;
	float refractionIndex;
	float fuzz;				// metalness
	float matte;			// how matte 
	float padding;		// <---- adding padding for alignment
};

StructuredBuffer<Material> g_Materials : register(t0);


float3 GetCameraPositionFromViewMatrix(float4x4 inverseViewMatrix) {
	// Extract the translation part from the inverse view matrix
	return mul(viewI, float4(0, 0, 0, 1)).xyz;
}

	
// Retrieve hit world position.
float3 HitWorldPosition()
{
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

[shader("closesthit")] void SphereClosestHit(inout HitInfo payload,
	SphereAttributes attrib) {

	if (payload.depth == 0)
	{
		payload.colorAndDistance = float4(0.0f,0.0f,0.0f, RayTCurrent());
		return;
	}
	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	float3 D = normalize(WorldRayDirection());
	float3 N = normalize(attrib.normal);

	float3 diffC = g_Materials[InstanceID()].diffuseColor.rgb;

	uint seed = GenerateSeed(DispatchRaysIndex().xy, elapsedTime, payload.colorAndDistance.w);


	RayDesc ray;
	ray.Origin = HitWorldPosition();
	ray.TMin = 0.01;
	ray.TMax = 100000;

	HitInfo newPayload;
	newPayload.colorAndDistance = float4(0, 0, 0, 0);
	newPayload.depth = payload.depth - 1;


	bool diffuse = g_Materials[InstanceID()].matte < 3.0;
	bool metal = !diffuse && g_Materials[InstanceID()].matte < 0.7;
	bool glass = !metal;




	if (diffuse)
	{
		seed = GenerateSeed(float2(seed, g_Materials[InstanceID()].matte), elapsedTime, payload.colorAndDistance.w);
		float3 scatter_direction = random_on_hemisphere(N, seed) + N;

		ray.Direction = scatter_direction;

		TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, newPayload);
	}
	else if (metal)
	{
		seed = GenerateSeed(float2(elapsedTime, seed), elapsedTime, payload.colorAndDistance.w);
		float3 scatter_direction = reflect(D, N) + random_on_hemisphere(N, seed) * g_Materials[InstanceID()].fuzz;

		ray.Direction = scatter_direction;

		TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, newPayload);
	}
	else if (glass)
	{
		float refractionIndex = g_Materials[InstanceID()].refractionIndex;
		if (dot(N, D) > 0.0f) {
			refractionIndex = 1.0f / refractionIndex;
			N = -N;
		}

		float cos_theta = min(dot(-D, N), 1.0);
		float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		seed = GenerateSeed(float2(43211, 532364213), elapsedTime, payload.colorAndDistance.w);

		float3 direction;

		if (refractionIndex * sin_theta > 1.0 || reflectance(cos_theta, refractionIndex) > GetRandomFloat(seed, 0.f, .1f))
		{
			direction = reflect(D, N);
		}
		else
		{
			direction = refract(D, N, refractionIndex);
		}

		ray.Direction = direction;

		TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, newPayload);
	}

	payload.colorAndDistance = float4(clamp(newPayload.colorAndDistance.rgb * (1.0f - g_Materials[InstanceID()].emissiveness) + diffC * g_Materials[InstanceID()].emissiveness, 0.0f, 1.0f), RayTCurrent());
}

