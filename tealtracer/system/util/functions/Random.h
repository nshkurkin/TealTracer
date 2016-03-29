///
///
///
///
///
///

#ifndef ____util_Random__
#define ____util_Random__

#include <random>
#include <src/util/util_base.h>

namespace util {

    /// Random represents a wrapper around a std random number generator. It
    /// takes a more Java API approach to creating numbers and supports random
    /// Eigen vectors.
    class Random {
    private:
    
        unsigned int seed;
    public:
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;
        
        
        /// Creates a random number generator by seeding a default random
        /// generator for a uniform integer distribution.
        Random(unsigned int seed = 0);
        /// Returns the seed set for this generator.
        unsigned int getSeed();
        
        /// Returns the next int between INT_MIN and INT_MAX
        int nextInt();
        /// Returns the next unsigned int between 0 and UINT_MAX
        unsigned int nextUInt();
        /// Returns a double of precision |precision| between 0 and 1.
        double nextDouble(int precision = 1000000);
        /// Returns a float of precision |precision| between 0 and 1.
        float nextFloat(int precision = 1000);
        
        /// Returns a random vector of size |n| with precision |precision|.
        /// Each of the vector's components will be between 0 and 1.
        template <int n>
        Eigen::Matrix<float, n, 1> nextVec(int precision = 1000);
    };
}

namespace util {
    template <int n>
    Eigen::Matrix<float, n, 1> Random::nextVec(int precision) {
        Eigen::Matrix<float, n, 1> toRet;
        for (int i = 0; i < n; i++)
            toRet[i] = nextFloat(precision);
        return toRet;
    }
}

#endif
