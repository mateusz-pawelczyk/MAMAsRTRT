#include "Common.hlsl"

// Raytracing output texture, accessed as a UAV
RWTexture2D<float4> gOutput : register(u0);
RWTexture2D<float4> gAccumulatedOutput : register(u1);
cbuffer frameIndex : register(b1)
{

	float frameIndex;
	bool cameraMoved;

};
// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0);

// #DXR Extra: Perspective Camera
cbuffer CameraParams : register(b0)
{
  float4x4 view;
  float4x4 projection;
  float4x4 viewI;
  float4x4 projectionI;
}



[shader("raygeneration")] void RayGen() {
  // Initialize the ray payload
  HitInfo payload;
  payload.colorAndDistance = float4(0, 0, 0, 0);
  payload.depth = 4;
  uint2 launchIndex = DispatchRaysIndex().xy;
  float2 dims = float2(DispatchRaysDimensions().xy);
  NumberGenerator ng;
  ng.seed = SetSeed(123456789);
  float samples_per_pixel = 32.f;


  float focusDistance = 5.f;
  // Perspective
  float3 color = float3(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < samples_per_pixel; ++i)
  {
	  

	  // Inside some shader logic, when you need a random value:
	  ng.seed = Cycle(ng.seed);
	  float randomValue = GetRandomFloat(ng.seed, 0.f, 1.f);

	  // Now use randomValue for your purposes


	  // Get the location within the dispatched 2D grid of work items
	  // (often maps to pixels, so this could represent a pixel coordinate).
	  
	  float2 d = (((launchIndex.xy + randomValue) / dims.xy) * 2.f - 1.f);
	  // Define a ray, consisting of origin, direction, and the min-max distance
	  // values
	  // #DXR Extra: Perspective Camera
	  float aspectRatio = dims.x / dims.y;
	  RayDesc ray;
	  ray.Origin = mul(viewI, float4(0, 0, 0, 1));
	  float4 target = mul(projectionI, float4(d.x, -d.y, 1, 1));
	  ray.Direction = mul(viewI, float4(target.xyz, 0));
	  ray.TMin = 0;
	  ray.TMax = 100000;


	  //float3 focusPoint = ray.Origin + ray.Direction * focusDistance;
	  //ng.seed = Cycle(ng.seed);

	  //float2 lensPoint = random_in_circle_using_angle(ng.seed) * 0.1f;
	  //float3 newOrigin = ray.Origin + float3(lensPoint.x, lensPoint.y, 0);
	  //float3 newDirection = normalize(focusPoint - newOrigin);
	  //ray.Origin = newOrigin;
	  //ray.Direction = newDirection;
	  //
	  //payload.colorAndDistance = float4(0, 0, 0, float(i));


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
		  payload);
	  color += payload.colorAndDistance.rgb;
  }
  color = sqrt(color / samples_per_pixel); // gamma correction

  if (cameraMoved == false && frameIndex > 0)
  {
	  gAccumulatedOutput[launchIndex] += float4(color , 1.f);

	  gOutput[launchIndex] = gAccumulatedOutput[launchIndex] / frameIndex;
  }
  else {
	  gOutput[launchIndex] = float4(color, 1.f);
	  gAccumulatedOutput[launchIndex] = float4(color, 1.f);//float4(.0f, .0f, .0f, 1.f);

  }

}
