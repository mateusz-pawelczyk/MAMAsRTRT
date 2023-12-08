#include "Common.hlsl"

// #DXR Extra - Another ray type
// Raytracing acceleration structure, accessed as a SRV
//RaytracingAccelerationStructure SceneBVH : register(t0, space1);



[shader("closesthit")] void SphereClosestHit(inout HitInfo payload,
	SphereAttributes attrib) {

	payload.colorAndDistance = float4(1.0f, 0.0f, 0.0f, RayTCurrent());;

}

