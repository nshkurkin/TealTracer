//
//  TSRandomValueGenerator.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/11/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSRandomValueGenerator_hpp
#define TSRandomValueGenerator_hpp

#include <random>

struct TSRandomValueGenerator {

    std::random_device randomDevice;
    std::mt19937 generator;
    
    std::uniform_real_distribution<float> floatDistribution;
    std::uniform_real_distribution<double> doubleDistribution;
    std::uniform_int_distribution<unsigned int> uintDistribution;

    TSRandomValueGenerator() {
        generator = std::mt19937(randomDevice());
        floatDistribution = std::uniform_real_distribution<float>(0.0,1.0);
        doubleDistribution = std::uniform_real_distribution<double>(0.0,1.0);
        uintDistribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max() - 1);
    }
    
    /// Generates a random float in the interval [0.0, 1.0]
    float randFloat() {
        return floatDistribution(generator);
    }
    
    /// Generates a random double in the interval [0.0, 1.0]
    double randDouble() {
        return doubleDistribution(generator);
    }
    
    /// Generates a random unsigned integer
    int randUInt() {
        return uintDistribution(generator);
    }
};

#endif /* TSRandomValueGenerator_hpp */
