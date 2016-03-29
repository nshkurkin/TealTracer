///
/// This is not my own source code. It is available at:
///   https://github.com/sol-prog/Perlin_Noise
///

#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include <src/util/util_base.h>
#include <src/util/util_types.h>

#ifndef ____util_PerlinNoise__
#define ____util_PerlinNoise__

namespace util {

    /// Represents a perlin noise generator. Given a seed, it produces a
    /// continuous noice imiplict function that can be smapled with any 3D
    /// point in space.
    ///
    /// NOTE: This C++ implementation has been rewritten to remove any C++11
    /// dependencies. This is useful for those who wish to stay C++98 compliant.
    ///
    /// The original C++ source should be available here:
    ///   https://github.com/sol-prog/Perlin_Noise
    ///
    /// From Original Author:
    /// THIS CLASS IS A TRANSLATION TO C++11 FROM THE REFERENCE
    /// JAVA IMPLEMENTATION OF THE IMPROVED PERLIN FUNCTION
    /// (see http://mrl.nyu.edu/~perlin/noise/)
    ///
    /// THE ORIGINAL JAVA IMPLEMENTATION IS COPYRIGHT 2002 KEN PERLIN
    ///
    /// I ADDED AN EXTRA METHOD THAT GENERATES A NEW PERMUTATION VECTOR
    /// (THIS IS NOT PRESENT IN THE ORIGINAL IMPLEMENTATION)
    class PerlinNoise {
    private:
        /// The permutation vector used for generating values.
        std::vector<int> p;
    public:
        unsigned int seed;
        /// Initialize with the reference values for the permutation vector
        PerlinNoise();
        /// Generate a new permutation vector based on the value of `seed`.
        PerlinNoise(unsigned int seed);
        /// Get a noise value, for 2D images z can have any value.
        double noise_double(double x, double y, double z);
        
        /// Returns the noise of (x, M_PI, M_PI).
        float noise(float x);
        /// Returns the noise of (x, y, M_PI).
        float noise(float x, float y);
        /// Returns a float casted result of util::PerlinNoise.noise_double.
        float noise(float x, float y, float z);
        
        struct CompoundNoiseConfig {
            int numSamples, octaveStart;
            float startingValue, startingFactor, noiseReduction, octaveFactor;
            
            CompoundNoiseConfig();
        };
        
        float compoundNoise(float x, float y, CompoundNoiseConfig cfg);

        
        float operator()(float x) {noise(x);}
        float operator()(float x, float y) {noise(x, y);}
        float operator()(float x, float y, float z) {noise(x, y, z);}
        
        /// Returns the noise associated with any arbitrary vector up to
        /// three components. Note that if the vector has more than three
        /// components, then excess values are ignored. Vectors with less than
        /// three components will automatically get M_PI filled into missing
        /// slots.
        template <typename T, int n>
        T noise(Eigen::Matrix<T, n, 1> vecn);
        
        template <typename T, int n>
        T operator ()(Eigen::Matrix<T, n, 1> vecn);
        
        
    private:
        double fade(double t);
        double lerp(double t, double a, double b);
        double grad(int hash, double x, double y, double z);
        void shuffle(unsigned int seed);
        void setupPermutationVector();
    };
}

namespace util {
    template <typename T, int n>
    T PerlinNoise::noise(Eigen::Matrix<T, n, 1> vecn) {
        Eigen::Matrix<T, 3, 1> maxv(T(M_PI), T(M_PI), T(M_PI));
        if (n <= 3)
            maxv.segment(0, n) = vecn.segment(0, n);
        else
            maxv.segment(0, 3) = vecn.segment(0, 3);
        return T(noise(float(maxv.x()), float(maxv.y()), float(maxv.z())));
    }
    
    template <typename T, int n>
    T PerlinNoise::operator()(Eigen::Matrix<T, n, 1> vecn) {
        return noise<T, n>(vecn);
    }
}

#endif //
