
// Ray payload for the shadow rays
struct ShadowHitInfo {
	bool isHit;
};

struct SphereAttributes
{
	float2 uv;
	float3 normal;
};

[shader("closesthit")] void ShadowSphereClosestHit(inout ShadowHitInfo hit,
	SphereAttributes bary) {
	hit.isHit = true;
}

[shader("miss")] void ShadowSphereMiss(inout ShadowHitInfo hit
	: SV_RayPayload) {
	hit.isHit = false;
}