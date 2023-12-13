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
	float padding[2];		// <---- adding padding for alignment
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

[shader("closesthit")] void SphereClosestHit(inout HitInfo payload, SphereAttributes attrib) {
	// Early exit for max depth
	if (payload.depth == 0) {
		payload.colorAndDistance = float4(0.0f, 0.0f, 0.0f, RayTCurrent());
		return;
	}

	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	float3 hitPosition = HitWorldPosition();

	float3 rayDirection = normalize(WorldRayDirection());
	float3 lightDir = hitPosition - lightPosition;
	float3 normal = normalize(attrib.normal);
	Material material = g_Materials[InstanceID()];
	uint seed = GenerateSeed(DispatchRaysIndex().xy, elapsedTime, payload.colorAndDistance.w);

	RayDesc ray;
	ray.Origin = hitPosition + normal * 0.01; // Offset to prevent self-intersection
	ray.TMin = 0.01;
	ray.TMax = 100000;

	float3 accumulatedColor = float3(0, 0, 0);

	// Ambient color component
	float3 ambientColor = material.diffuseColor.rgb * 0.1; // Simulate some ambient light

	// Emissive Material
	if (material.emissiveness > 0.0) {
		accumulatedColor += material.emissiveColor.rgb * material.emissiveness;
	}



	// Diffuse Reflection
	if (material.emissiveness > 0.0) {
		seed = GenerateSeed(float2(seed, material.matte), elapsedTime, payload.colorAndDistance.w);
		float3 scatterDir = random_on_hemisphere(normal, seed);
		ray.Direction = scatterDir;
		HitInfo diffusePayload;
		diffusePayload.colorAndDistance = float4(0, 0, 0, 0);
		diffusePayload.depth = payload.depth - 1;
		TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, diffusePayload);
		float NdotL = max(dot(normal, -lightDir), 0.0);

		accumulatedColor += diffusePayload.colorAndDistance.rgb * material.diffuseColor.rgb;
	}

	// Specular Reflection (Metal)
	if (material.reflectivity > 0.0) {
		seed = GenerateSeed(float2(elapsedTime, seed), elapsedTime, payload.colorAndDistance.w);
		float3 reflectedDir = reflect(rayDirection, normal);
		float3 fuzzDir = random_on_hemisphere(normal, seed) * material.fuzz;
		ray.Direction = normalize(reflectedDir + fuzzDir);
		HitInfo metalPayload;
		metalPayload.colorAndDistance = float4(0, 0, 0, 0);
		metalPayload.depth = payload.depth - 1;
		TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, metalPayload);
		accumulatedColor += metalPayload.colorAndDistance.rgb * material.reflectivity + material.specularColor.rgb * material.emissiveness;
	}

	// Refraction (Glass)
	if (material.refractivity > 0.0) {
		float refractionIndex = material.refractionIndex;
		bool entering = dot(normal, rayDirection) < 0.0f;
		float3 refractedDir = refract(rayDirection, entering ? normal : -normal, entering ? 1.0f / refractionIndex : refractionIndex);
		ray.Direction = refractedDir;
		HitInfo glassPayload;
		glassPayload.colorAndDistance = float4(0, 0, 0, 0);
		glassPayload.depth = payload.depth - 1;
		TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, glassPayload);
		float3 refractedColor = glassPayload.colorAndDistance.rgb; // This should be the color from the refracted ray
		accumulatedColor = lerp(accumulatedColor, refractedColor, material.refractivity);
	}

	// Add ambient component to final color
	accumulatedColor += ambientColor;

	// Final color calculation
	payload.colorAndDistance = float4(clamp(accumulatedColor, 0.0f, 1.0f), RayTCurrent());
}