///
/// This is not my own source code. It is available at:
///   https://github.com/sol-prog/Perlin_Noise
///
/// See README.md for more information.
///
/// NOTE: This code did not compile for me at first, specifically line 25 used
/// to try and statically define a std::vector<int> using an initializer list.
/// The solution was found here:
///   http://stackoverflow.com/questions/2236197/how-to-initialize-an-stl-vector-with-hardcoded-elements-in-the-easiest-way
/// As noted in the .h file, I rewrote part of the constructor to properly
/// support c++98

#include "PerlinNoise.h"
#include "Random.h"

using namespace util;

// THIS IS A DIRECT TRANSLATION TO C++11 FROM THE REFERENCE
// JAVA IMPLEMENTATION OF THE IMPROVED PERLIN FUNCTION
// (see http://mrl.nyu.edu/~perlin/noise/)
//
// THE ORIGINAL JAVA IMPLEMENTATION IS COPYRIGHT 2002 KEN PERLIN

// I ADDED AN EXTRA METHOD THAT GENERATES A NEW PERMUTATION VECTOR
// (THIS IS NOT PRESENT IN THE ORIGINAL IMPLEMENTATION)

// Initialize with the reference values for the permutation vector
PerlinNoise::PerlinNoise() {
	seed = 0;
    setupPermutationVector();
	shuffle(seed);
}

// Generate a new permutation vector based on the value of seed
PerlinNoise::PerlinNoise(unsigned int seed) {
	this->seed = seed;
    setupPermutationVector();
	shuffle(seed);
}

void PerlinNoise::setupPermutationVector() {
   int filler = -1;
   p.resize(256);

	// Fill p with values from 0 to 255
   while (++filler < 256)
      p[filler] = filler;
}

void PerlinNoise::shuffle(unsigned int seed) {
   // Initialize a random engine with seed
    Random generator(seed);
	//srand(seed);
   
	// Suffle  using the above random engine
   std::vector<int>::iterator first = p.begin(), last = p.end();
   int i = (last - first) - 1;
   
   while (i > 0) {
      std::swap(first[i], first[generator.nextUInt() % i]);
      --i;
   }

	// Duplicate the permutation vector
	p.insert(p.end(), p.begin(), p.end());
}

float PerlinNoise::noise(float x) {
    return noise(x, (float) M_PI, (float) M_PI);
}
float PerlinNoise::noise(float x, float y) {
    return noise(x, y,(float) M_PI);
}
float PerlinNoise::noise(float x, float y, float z) {
    return (float) noise_double(x, y, z);
}

double PerlinNoise::noise_double(double x, double y, double z) {
	// Find the unit cube that contains the point
	int X = (int) floor(x) & 255;
	int Y = (int) floor(y) & 255;
	int Z = (int) floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res
     = lerp(w,
      lerp(v,
         lerp(u,
            grad(p[AA], x, y, z),
            grad(p[BA], x-1, y, z)),
         lerp(u,
            grad(p[AB], x, y-1, z),
            grad(p[BB], x-1, y-1, z))),
      lerp(v,
         lerp(u,
            grad(p[AA+1], x, y, z-1),
            grad(p[BA+1], x-1, y, z-1)),
         lerp(u,
            grad(p[AB+1], x, y-1, z-1),
            grad(p[BB+1], x-1, y-1, z-1))));
   
	return (res + 1.0)/2.0;
}

PerlinNoise::CompoundNoiseConfig::CompoundNoiseConfig() {
    numSamples = 1;
    octaveStart = 0;
    startingValue = 0.0f;
    startingFactor = 0.0f;
    noiseReduction = 1.0f;
    octaveFactor = 2.0f;
}

float PerlinNoise::compoundNoise(float x, float y,
PerlinNoise::CompoundNoiseConfig cfg) {
    auto specialNoise = [=] (float x, float y, float factor) {
        static float piSqrd = (float) (M_PI * M_PI);
        return (float) (this->noise(
            x * factor + piSqrd,
            y * factor + piSqrd,
            1 * factor + piSqrd
        ) / factor);
    };

    int whichOctave = cfg.octaveStart;
    float value = cfg.startingValue, currFactor = cfg.startingFactor;
    while (++whichOctave <= cfg.numSamples)
        value += specialNoise(x, y, (currFactor *= cfg.octaveFactor)) / cfg.noiseReduction;
    return value;
}

double PerlinNoise::fade(double t) { 
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) { 
	return a + t * (b - a); 
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	// Convert lower 4 bits of hash inot 12 gradient directions
	double u = h < 8 ? x : y,
		   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
