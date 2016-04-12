//
//  PhotonMap.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PhotonMap.hpp"

///
float PhotonMap::gaussianWeight(float distSqrd, float radius) {
    static const float oneOverSqrtTwoPi = 0.3989422804f;
    
    float sigma = radius/3.0;
    return (oneOverSqrtTwoPi / sigma) * exp( - (distSqrd) / (2.0 * sigma * sigma) );
}

///
float PhotonMap::gaussianWeightJensen(float distSqrd, float radius) {
    static const float alpha = 0.918f;
    static const float beta  = 1.953f;

    return alpha * (1.0f - (1.0f - exp(-beta * distSqrd / (2.0f * radius * radius))) / (1.0f - std::exp(-beta))) ;
}
