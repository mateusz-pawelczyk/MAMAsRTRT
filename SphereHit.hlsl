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
	float specularPower;
	float emissiveness;
	float reflectivity;
	float refractivity;
	float refractionIndex;
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
		payload.colorAndDistance = float4(0, 0, 0, RayTCurrent());
		return;
	}
	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	float3 D = normalize(WorldRayDirection());

	// reflection
	RayDesc ray;
	ray.Origin = HitWorldPosition();
	ray.Direction = normalize(-D);
	ray.TMin = 0.01;
	ray.TMax = 100000;

	// Initialize the ray payload
	HitInfo reflectionPayload;
	reflectionPayload.colorAndDistance = float4(0, 0, 0, 0);
	reflectionPayload.depth = payload.depth - 1;



	// Trace the ray
	TraceRay(
		// Parameter name: AccelerationStructure
		// Acceleration structure
		SceneBVH,

		// Parameter name: RayFlags
		// Flags can be used to specify the behavior upon hitting a surface
		RAY_FLAG_NONE,

		// Parameter name: InstanceInclusionMask
		// Instance inclusion mask, which can be used to mask out some geometry to
		// this ray by and-ing the mask with a geometry mask. The 0xFF flag then
		// indicates no geometry will be masked
		0xFF,

		// Parameter name: RayContributionToHitGroupIndex
		// Depending on the type of ray, a given object can have several hit
		// groups attached (ie. what to do when hitting to compute regular
		// shading, and what to do when hitting to compute shadows). Those hit
		// groups are specified sequentially in the SBT, so the value below
		// indicates which offset (on 4 bits) to apply to the hit groups for this
		// ray. In this sample we only have one hit group per object, hence an
		// offset of 0.
		0,

		// Parameter name: MultiplierForGeometryContributionToHitGroupIndex
		// The offsets in the SBT can be computed from the object ID, its instance
		// ID, but also simply by the order the objects have been pushed in the
		// acceleration structure. This allows the application to group shaders in
		// the SBT in the same order as they are added in the AS, in which case
		// the value below represents the stride (4 bits representing the number
		// of hit groups) between two consecutive objects.
		0,

		// Parameter name: MissShaderIndex
		// Index of the miss shader to use in case several consecutive miss
		// shaders are present in the SBT. This allows to change the behavior of
		// the program when no geometry have been hit, for example one to return a
		// sky color for regular rendering, and another returning a full
		// visibility value for shadow rays. This sample has only one miss shader,
		// hence an index 0
		0,

		// Parameter name: Ray
		// Ray information to trace
		ray,

		// Parameter name: Payload
		// Payload associated to the ray, which will be used to communicate
		// between the hit/miss shaders and the raygen
		reflectionPayload);

	float3 color = float3(1.f, 1.f, 1.f);
	
	if (PrimitiveIndex() == 0)
		color = float3(0.f, 1.f, 0.f);
	if (PrimitiveIndex() == 1)
		color = float3(0.f, 0.f, 1.f);
	if (PrimitiveIndex() == 2)
		color = float3(0.f, 1.f, 1.f);
	if (PrimitiveIndex() == 3)
		color = float3(1.f, 1.f,0.f);
	if (PrimitiveIndex() == 4)
		color = float3(1.f, 1.f, 0.f);
	if (PrimitiveIndex() == 5)
		color = float3(1.f, 1.f, 1.f);
	if (g_Materials[0].specularColor.w >= 0.7)
	{
		payload.colorAndDistance = float4(g_Materials[1].diffuseColor.xyz, RayTCurrent());

	}
	else {
		payload.colorAndDistance = float4(g_Materials[0].diffuseColor.xyz, RayTCurrent());

	}

	
	//normalize(normalize(float3(0.0f, elapsedTime, 0.0f)) + normalize(float3(0.0f, 0.0f , 1 - elapsedTime)))
}

