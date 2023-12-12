// Hit information, aka ray payload
// This sample only carries a shading color and hit distance.
// Note that the payload should be kept as small as possible,
// and that its size must be declared in the corresponding
// D3D12_RAYTRACING_SHADER_CONFIG pipeline subobjet.
struct HitInfo {
  float4 colorAndDistance;
  int depth;
};

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes {
  float2 bary;
};


struct SphereAttributes {
	//float3 normal; // Surface normal at intersection
	float2 uv;     // Texture coordinates
	float3 normal;
};

#define RANDOM_IA 16807
#define RANDOM_IM 2147483647
#define RANDOM_AM (1.0/RANDOM_IM)
#define RANDOM_IQ 127773
#define RANDOM_IR 2836
#define RANDOM_MASK 123459876

// Represents the generator state.
struct NumberGenerator {
	int seed; // Used to generate values.
};

// Generates the next number in the sequence.
int Cycle(int seed) {
	seed ^= RANDOM_MASK;
	int k = seed / RANDOM_IQ;
	seed = RANDOM_IA * (seed - k * RANDOM_IQ) - RANDOM_IR * k;

	if (seed < 0)
		seed += RANDOM_IM;

	seed ^= RANDOM_MASK;
	return seed;
}

// Cycles the generator based on the input count. Useful for generating a thread unique seed.
int CycleCount(int seed, uint count) {
	for (uint i = 0; i < count; ++i)
		seed = Cycle(seed);
	return seed;
}

// Sets the seed
int SetSeed(uint value) {
	int seed = int(value);
	return Cycle(seed);
}

// Returns the current random float from the seed.
float GetCurrentFloat(int seed) {
	return RANDOM_AM * seed;
}

// Returns a random float within the input range.
float GetRandomFloat(int seed, float low, float high) {
	float v = GetCurrentFloat(seed);
	return low * (1.0f - v) + high * v;
}





// Constants
static const uint LARGE_PRIME1 = 198491317;
static const uint LARGE_PRIME2 = 6542989;
static const uint LARGE_PRIME3 = 7393931;

// Seed generation with elapsed time and ray index
uint GenerateSeed(uint2 pixelCoord, float elapsedTime, uint rayIndex) {
	uint timeHash = uint(elapsedTime * 1000.0f) * LARGE_PRIME1;
	return pixelCoord.x * LARGE_PRIME2 + pixelCoord.y * LARGE_PRIME1 + timeHash + rayIndex * LARGE_PRIME3;
}

// Hash-based random number generator
float HashRandom(inout uint seed) {
	seed ^= RANDOM_MASK;
	int k = seed / RANDOM_IQ;
	seed = RANDOM_IA * (seed - k * RANDOM_IQ) - RANDOM_IR * k;

	if (seed < 0)
		seed += RANDOM_IM;

	seed ^= RANDOM_MASK;
	seed += LARGE_PRIME2;
	return sin(frac(float(seed) / 100000.0f));
}

// Generate a single random vector within a specified range
float3 GenerateRandomVectorInRange(float minValue, float maxValue, inout uint seed) {
	float range = maxValue - minValue;

	float x = minValue + HashRandom(seed) * range;
	float y = minValue + HashRandom(seed) * range;
	float z = minValue + HashRandom(seed) * range;

	// Update the seed for the next call
	seed += 15731;// +rayIndex * 743;

	return float3(x, y, z);
}



float3 random_in_unit_sphere(inout uint seed) {
	uint max_iterations = 100;
	uint iterations = 0;

	float3 p = float3(0.0f, 0.0f, 0.0f);

	NumberGenerator ng;
	ng.seed = SetSeed(seed);
	p.x = GetRandomFloat(ng.seed, 0.f, 1.f);
	ng.seed = SetSeed(ng.seed);
	p.y = GetRandomFloat(ng.seed, 0.f, 1.f);
	ng.seed = SetSeed(ng.seed);
	p.z = GetRandomFloat(ng.seed, 0.f, 1.f);


	while (dot(p, p) >= 1 && iterations < max_iterations) {
		ng.seed = SetSeed(ng.seed);
		p.x = GetRandomFloat(ng.seed, 0.f, 1.f);
		ng.seed = SetSeed(ng.seed);
		p.y = GetRandomFloat(ng.seed, 0.f, 1.f);
		ng.seed = SetSeed(ng.seed);
		p.z = GetRandomFloat(ng.seed, 0.f, 1.f);

		iterations++;
	}

	seed = ng.seed;
	return normalize(p);
}

float3 random_on_hemisphere(float3 normal, inout uint seed) {
	float3 on_unit_sphere = random_in_unit_sphere(seed);
	if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
		return on_unit_sphere;
	else
		return -on_unit_sphere;
}