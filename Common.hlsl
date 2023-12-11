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