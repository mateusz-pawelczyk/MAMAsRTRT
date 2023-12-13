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

//struct Material {
//	float4 diffuseColor;
//	float4 specularColor;
//	float4 emissiveColor;
//	float emissiveness;		// how diffuse
//	float reflectivity;
//	float refractivity;
//	float refractionIndex;
//	float fuzz;				// metalness
//	float matte;			// how matte 
//	float padding[2];		// <---- adding padding for alignment
//};


struct  Material {
	float4 albedo;
	float refractionIndex;
	float fuzz;				// metalness
	float matte;			// how matte 
	float padding;			// <---- adding padding for alignment
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

	//float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	//float3 hitPosition = HitWorldPosition();

	//float3 rayDirection = normalize(WorldRayDirection());
	//float3 lightDir = hitPosition - lightPosition;
	//float3 normal = normalize(attrib.normal);
	//Material material = g_Materials[InstanceID()];
	//uint seed = GenerateSeed(DispatchRaysIndex().xy, elapsedTime, payload.colorAndDistance.w);

	//RayDesc ray;
	//ray.Origin = hitPosition + normal * 0.01; // Offset to prevent self-intersection
	//ray.TMin = 0.01;
	//ray.TMax = 100000;

	//float3 accumulatedColor = float3(0, 0, 0);

	//// Ambient color component
	//float3 ambientColor = material.diffuseColor.rgb * 0.1; // Simulate some ambient light

	//// Emissive Material
	//if (material.emissiveness > 0.0) {
	//	accumulatedColor += material.emissiveColor.rgb * material.emissiveness;
	//}



	//// Diffuse Reflection
	//if (material.emissiveness > 0.0) {
	//	seed = GenerateSeed(float2(seed, material.matte), elapsedTime, payload.colorAndDistance.w);
	//	float3 scatterDir = random_on_hemisphere(normal, seed) + normal;
	//	ray.Direction = scatterDir;
	//	HitInfo diffusePayload;
	//	diffusePayload.colorAndDistance = float4(0, 0, 0, 0);
	//	diffusePayload.depth = payload.depth - 1;
	//	TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, diffusePayload);
	//	// Adjusting for Lambertian reflectance
	//	float NdotL = max(dot(normal, -lightDir), 0.0);
	//	accumulatedColor += diffusePayload.colorAndDistance.rgb * material.diffuseColor.rgb * NdotL;	
	//}

	//// Specular Reflection (Metal)
	//if (material.reflectivity > 0.0) {
	//	seed = GenerateSeed(float2(elapsedTime, seed), elapsedTime, payload.colorAndDistance.w);
	//	float3 reflectedDir = reflect(rayDirection, normal);
	//	// Adjusting balance between reflected and fuzz direction
	//	float3 fuzzDir = random_on_hemisphere(normal, seed) * material.fuzz;
	//	ray.Direction = normalize(reflectedDir + fuzzDir * 0.5); // Reduced fuzz influence
	//	HitInfo metalPayload;
	//	metalPayload.colorAndDistance = float4(0, 0, 0, 0);
	//	metalPayload.depth = payload.depth - 1;
	//	TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, metalPayload);
	//	accumulatedColor += metalPayload.colorAndDistance.rgb * material.reflectivity + material.specularColor.rgb * material.emissiveness;
	//}

	//// Refraction (Glass)
	//if (material.refractivity > 0.0) {
	//	float refractionIndex = material.refractionIndex;
	//	if (dot(normal, rayDirection) > 0.0f) {
	//		refractionIndex = 1.0f / refractionIndex;
	//		normal = -normal;
	//	}

	//	float cos_theta = min(dot(-rayDirection, normal), 1.0);
	//	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	//	seed = GenerateSeed(float2(seed, elapsedTime), elapsedTime, payload.colorAndDistance.w);

	//	float3 direction;

	//	if (refractionIndex * sin_theta > 1.0 || reflectance(cos_theta, refractionIndex) > GetRandomFloat(seed, 0.f, 1.f))
	//	{
	//		direction = reflect(rayDirection, normal);
	//	}
	//	else
	//	{
	//		direction = refract(rayDirection, normal, refractionIndex);
	//	}
	//	ray.Direction = direction;

	//	HitInfo glassPayload;
	//	glassPayload.colorAndDistance = float4(0, 0, 0, 0);
	//	glassPayload.depth = payload.depth - 1;
	//	TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, glassPayload);
	//	float3 refractedColor = glassPayload.colorAndDistance.rgb; // This should be the color from the refracted ray
	//	accumulatedColor = refractedColor;// lerp(accumulatedColor, refractedColor, material.refractivity);
	//}

	//// Add ambient component to final color
	//accumulatedColor += ambientColor;

	// Final color calculation
	payload.colorAndDistance = float4(0.0, 0.0, 1.0, RayTCurrent());
}


[shader("closesthit")]
void SphereClosestHitLambertian(inout HitInfo payload, SphereAttributes attrib) 
{
	if (payload.depth == 0) {
		payload.colorAndDistance = float4(0.0f, 0.0f, 0.0f, RayTCurrent());
		return;
	}

	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);	
	float3 hitPosition = HitWorldPosition();	

	RayDesc ray;
	ray.Origin = hitPosition;
	ray.TMin = 0.1;
	ray.TMax = 100000;

	float3 D = normalize(WorldRayDirection());	// Ray Direction
	float3 L = hitPosition - lightPosition;		// Light Direction
	float3 N = normalize(attrib.normal);		// Normal
	Material material = g_Materials[InstanceID()];	// Material properties

	uint seed = GenerateSeed(DispatchRaysIndex().xy, elapsedTime, payload.colorAndDistance.w);	// Seed to generate random numbers


	float3 albedo = material.albedo.rgb;
	float3 scatter_direction = N + random_unit_vector(seed);

	// Catch degenerate scatter direction
	if (near_zero(scatter_direction))
		scatter_direction = N;

	ray.Direction = scatter_direction;

	HitInfo scatteredPayload;
	scatteredPayload.colorAndDistance = float4(0, 0, 0, 0);
	scatteredPayload.depth = payload.depth - 1;
	TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, scatteredPayload);

	payload.colorAndDistance = float4(albedo * scatteredPayload.colorAndDistance.rgb, RayTCurrent());
}



[shader("closesthit")]
void SphereClosestHitMetal(inout HitInfo payload, SphereAttributes attrib)
{
	if (payload.depth == 0) {
		payload.colorAndDistance = float4(0.0f, 0.0f, 0.0f, RayTCurrent());
		return;
	}

	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	float3 hitPosition = HitWorldPosition();

	RayDesc ray;
	ray.Origin = hitPosition;
	ray.TMin = 0.1;
	ray.TMax = 100000;

	float3 D = normalize(WorldRayDirection());	// Ray Direction
	float3 L = hitPosition - lightPosition;		// Light Direction
	float3 N = normalize(attrib.normal);		// Normal
	Material material = g_Materials[InstanceID()];	// Material properties

	uint seed = GenerateSeed(DispatchRaysIndex().xy, elapsedTime, payload.colorAndDistance.w);	// Seed to generate random numbers


	float3 albedo	= material.albedo.rgb;
	float  fuzz		= material.fuzz < 1.0f ? material.fuzz : 1.0f;

	float3 reflected = reflect(D, N);
	float3 scatter_direction = reflected + fuzz * random_in_unit_sphere(seed);
	ray.Direction = scatter_direction;

	HitInfo scatteredPayload;
	scatteredPayload.colorAndDistance = float4(0, 0, 0, 0);
	scatteredPayload.depth = payload.depth - 1;
	TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, scatteredPayload);

	float3 color;
	if (dot(scatter_direction, N) > 0)
	{
		color = albedo * scatteredPayload.colorAndDistance.rgb;
	}
	else
	{
		color = float3(0.0f, 0.0f, 0.0f);
	}

	payload.colorAndDistance = float4(color, RayTCurrent());
}


[shader("closesthit")]
void SphereClosestHitDielectric(inout HitInfo payload, SphereAttributes attrib)
{
	if (payload.depth == 0) {
		payload.colorAndDistance = float4(0.0f, 0.0f, 0.0f, RayTCurrent());
		return;
	}

	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	float3 hitPosition = HitWorldPosition();

	RayDesc ray;
	ray.Origin = hitPosition;
	ray.TMin = 0.1;
	ray.TMax = 100000;

	float3 D = normalize(WorldRayDirection());	// Ray Direction
	float3 L = hitPosition - lightPosition;		// Light Direction
	float3 N = normalize(attrib.normal);		// Normal
	Material material = g_Materials[InstanceID()];	// Material properties

	uint seed = GenerateSeed(DispatchRaysIndex().xy, elapsedTime, payload.colorAndDistance.w);	// Seed to generate random numbers


	float  eta  = material.refractionIndex;

	if (dot(N, D) > 0.0f) {
		eta = 1.0f / eta;
		N = -N;
	}

	double cos_theta = min(dot(-D, N), 1.0);
	double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	bool cannot_refract = eta * sin_theta > 1.0;
	float3 direction;

	if (cannot_refract || reflectance(cos_theta, eta) > GetRandomFloat(seed, 0.f, 1.f))
	{
		direction = reflect(D, N);
	}
	else
	{
		direction = refract(D, N, eta);
	}

	float3 scatter_direction = direction;
	ray.Direction = scatter_direction;

	HitInfo scatteredPayload;
	scatteredPayload.colorAndDistance = float4(0, 0, 0, 0);
	scatteredPayload.depth = payload.depth - 1;
	TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, scatteredPayload);

	float3 color = scatteredPayload.colorAndDistance.rgb;
	
	payload.colorAndDistance = float4(color, RayTCurrent());
}