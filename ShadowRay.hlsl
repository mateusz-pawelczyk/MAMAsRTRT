// #DXR Extra - Another ray type
#include "Common.hlsl"
// Ray payload for the shadow rays
struct ShadowHitInfo {
  bool isHit;
};


[shader("closesthit")] void ShadowClosestHit(inout ShadowHitInfo hit,
                                             SphereAttributes bary) {
  hit.isHit = true;
}

[shader("miss")] void ShadowMiss(inout ShadowHitInfo hit
                                  : SV_RayPayload) {
  hit.isHit = false;
}