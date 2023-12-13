#include "Common.hlsl"

[shader("miss")] void Miss(inout HitInfo payload
                           : SV_RayPayload) {
  float3 D = normalize(WorldRayDirection());	// Ray Direction

  float3 a = 0.5 * (D.y + 1.0);

  payload.colorAndDistance = float4((1.0 - a) * float3(1.0, 1.0, 1.0) + a * float3(0.5, 0.7, 1.0), -1.0f);
}