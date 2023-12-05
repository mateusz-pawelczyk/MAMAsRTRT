#include "Common.hlsl"

// #DXR Extra - Another ray type
struct ShadowHitInfo {
  bool isHit;
};

// #DXR Extra - Another ray type
// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t2);

struct STriVertex {
  float3 vertex;
  float4 color;
  float3 normal;   // Normal vector
  float2 texcoord; // Texture coordinates
  float3 tangent;  // Tangent vector
  float3 binormal; // Binormal vector
};

// #DXR Extra: Per-Instance Data
cbuffer Colors : register(b0) {
  float3 A;
  float3 B;
  float3 C;
}



cbuffer CameraParams : register(b1) {
	float4x4 view;
	float4x4 projection;
	float4x4 viewI;       // Inverse of the view matrix
	float4x4 projectionI; // Inverse of the projection matrix
}



StructuredBuffer<STriVertex> BTriVertex : register(t0);
StructuredBuffer<int> indices : register(t1);


static const float4 backgroundColor = float4(0.4, 0.6, 0.2, 1.0);
static const float4 lightAmbientColor = float4(0.2, 0.2, 0.2, 1.0);
static const float3 lightPosition = float3(3.0, 3.0, 3.0);
static const float4 lightDiffuseColor = float4(0.5, 0.5, 0.5, 1.0);
static const float4 lightSpecularColor = float4(1, 1, 1, 1);
static const float4 primitiveAlbedo = float4(1.0, 0.0, 0.0, 1.0);
static const float diffuseCoef = 0.9;
static const float specularCoef = 0.3;
static const float specularPower = 50;
static const float refractiveness = 0.0f;
static const float reflectiveness = 0.6f;


float3 GetCameraPositionFromViewMatrix(float4x4 inverseViewMatrix) {
	// Extract the translation part from the inverse view matrix
	return mul(viewI, float4(0, 0, 0, 1)).xyz;
}


// Retrieve hit world position.
float3 HitWorldPosition()
{
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Diffuse lighting calculation.
float CalculateDiffuseCoefficient(in float3 hitPosition, in float3 incidentLightRay, in float3 normal)
{
	float fNDotL = saturate(dot(-incidentLightRay, normal));
	return fNDotL;
}

// Phong lighting specular component
float4 CalculateSpecularCoefficient(in float3 hitPosition, in float3 incidentLightRay, in float3 normal, in float specularPower)
{
	float3 reflectedLightRay = normalize(reflect(incidentLightRay, normal));
	return pow(saturate(dot(reflectedLightRay, normalize(-WorldRayDirection()))), specularPower);
}

// Phong lighting model = ambient + diffuse + specular components.
float4 CalculatePhongLighting(in float4 albedo, in float3 normal, in float diffuseCoef = 1.0, in float specularCoef = 1.0, in float specularPower = 50)
{
	float3 cameraPos = GetCameraPositionFromViewMatrix(viewI);
	float3 hitPosition = HitWorldPosition();
	float3 lightDirection = normalize(lightPosition - hitPosition);

	// Ambient component
	float4 ambientColor =  0.2 * albedo;

	// Diffuse component
	float diffuseFactor = max(dot(normal, lightDirection), 0.0f);
	float4 diffuseColor = diffuseCoef * diffuseFactor * lightDiffuseColor * albedo;

	// Specular component
	float3 viewDirection = normalize(cameraPos - hitPosition);
	float3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularFactor = pow(max(dot(reflect(-lightDirection, normal), viewDirection), 0.0f), specularPower);
	float4 specularColor = specularCoef * specularFactor * lightSpecularColor;

	// Combine all components
	float4 finalColor = ambientColor + diffuseColor + specularColor;
	return finalColor;
}

float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
	return vertexAttribute[0] +
		attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

[shader("closesthit")] void ClosestHit(inout HitInfo payload,
                                       Attributes attrib) {
	if (payload.depth == 0)
	{
		payload.colorAndDistance = float4(0,0,0, RayTCurrent());
		return;
	}
	float3 barycentrics =
		float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

	uint vertId = 3 * PrimitiveIndex();

	// Retrieve corresponding vertex normals for the triangle vertices.
	float3 vertexNormals[3] = {
		BTriVertex[indices[vertId + 0]].normal,
		BTriVertex[indices[vertId + 1]].normal,
		BTriVertex[indices[vertId + 2]].normal
	};


	float3 hitNormal = HitAttribute(vertexNormals, attrib);

	
	
	// #DXR Extra: Per-Instance Data
	// interpolating the color between vertices because each vertex has a colorn assigned in STriVertex.
	float3 hitColor = BTriVertex[indices[vertId + 0]].color * barycentrics.x +
		BTriVertex[indices[vertId + 1]].color * barycentrics.y +
		 barycentrics.z ;
	

	// Calculate final color.
	float4 phongColor = CalculatePhongLighting(float4(0.3, 0.6, 0.1, 1.0f), hitNormal, diffuseCoef, specularCoef, specularPower);

	 float3 D = normalize(WorldRayDirection());
	 float3 N = normalize(hitNormal);

	// reflection
	RayDesc ray;
	ray.Origin = HitWorldPosition();
	ray.Direction = normalize(reflect(D, N));
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



	float eta = 1.0f / 1.3;
	if (dot(N, D) > 0) {
		eta = 1.0f / eta;
		N = -N;
	}

	float r = dot(-D, N);
	float c = 1.0f - eta * eta * (1 - r * r);
	float3 R = eta * D + (eta * r - sqrt(c)) * N;

	// refraction
	RayDesc rayRefract;
	rayRefract.Origin = HitWorldPosition();
	rayRefract.Direction = normalize(R);
	rayRefract.TMin = 0.1;
	rayRefract.TMax = 100000;

	// Initialize the ray payload
	HitInfo refractionPayload;
	refractionPayload.colorAndDistance = float4(0, 0, 0, 0);
	refractionPayload.depth = payload.depth - 1;


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
		rayRefract,

		// Parameter name: Payload
		// Payload associated to the ray, which will be used to communicate
		// between the hit/miss shaders and the raygen
		refractionPayload);



	
	payload.colorAndDistance = float4(reflectiveness * reflectionPayload.colorAndDistance.xyz + refractiveness * refractionPayload.colorAndDistance.xyz +(1-reflectiveness-refractiveness) * phongColor.xyz, RayTCurrent());;
}

// #DXR Extra - Another ray type
[shader("closesthit")] void PlaneClosestHit(inout HitInfo payload,
                                                Attributes attrib) {
  float3 lightPos = float3(2, 2, -2);

  // Find the world - space hit position
  float3 worldOrigin = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

  float3 lightDir = normalize(lightPos - worldOrigin);

  // Fire a shadow ray. The direction is hard-coded here, but can be fetched
  // from a constant-buffer
  RayDesc ray;
  ray.Origin = worldOrigin;
  ray.Direction = lightDir;
  ray.TMin = 0.01;
  ray.TMax = 100000;
  bool hit = true;

  // Initialize the ray payload
  ShadowHitInfo shadowPayload;
  shadowPayload.isHit = false;

  // Trace the ray
  TraceRay(
      // Acceleration structure
      SceneBVH,
      // Flags can be used to specify the behavior upon hitting a surface
      RAY_FLAG_NONE,
      // Instance inclusion mask, which can be used to mask out some geometry to
      // this ray by and-ing the mask with a geometry mask. The 0xFF flag then
      // indicates no geometry will be masked
      0xFF,
      // Depending on the type of ray, a given object can have several hit
      // groups attached (ie. what to do when hitting to compute regular
      // shading, and what to do when hitting to compute shadows). Those hit
      // groups are specified sequentially in the SBT, so the value below
      // indicates which offset (on 4 bits) to apply to the hit groups for this
      // ray. In this sample we only have one hit group per object, hence an
      // offset of 0.
      1,
      // The offsets in the SBT can be computed from the object ID, its instance
      // ID, but also simply by the order the objects have been pushed in the
      // acceleration structure. This allows the application to group shaders in
      // the SBT in the same order as they are added in the AS, in which case
      // the value below represents the stride (4 bits representing the number
      // of hit groups) between two consecutive objects.
      0,
      // Index of the miss shader to use in case several consecutive miss
      // shaders are present in the SBT. This allows to change the behavior of
      // the program when no geometry have been hit, for example one to return a
      // sky color for regular rendering, and another returning a full
      // visibility value for shadow rays. This sample has only one miss shader,
      // hence an index 0
      1,
      // Ray information to trace
      ray,
      // Payload associated to the ray, which will be used to communicate
      // between the hit/miss shaders and the raygen
      shadowPayload);

  float factor = shadowPayload.isHit ? 0.3 : 1.0;

  float3 barycentrics =
      float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);
  
  float4 hitColor = float4(float3(1.0, 0.0, 0.0) * factor, RayTCurrent());
  payload.colorAndDistance = float4(hitColor);
}