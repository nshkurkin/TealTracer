//
//  PhotonMap.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PhotonMap_hpp
#define PhotonMap_hpp

#include <vector>
#include <Eigen/Dense>

#include "JensenPhoton.hpp"

///
class PhotonMap {
public:
    PhotonMap() {}
    virtual ~PhotonMap() {}
    
    std::vector<JensenPhoton> photons;
    
    /// Call this after filling "photons" with the relevant content.
    virtual void buildMap() = 0;
    
    ///
    struct PhotonIndexInfo {
        int index;
        float squareDistance;
        
        PhotonIndexInfo() : index(0), squareDistance(0) {}
        PhotonIndexInfo(int index, float sqrDist) : index(index), squareDistance(sqrDist) {}
    };
    
    /// Call this after building the spatial hash.
    virtual std::vector<PhotonIndexInfo> gatherPhotonsIndices(
        int maxNumPhotonsToGather,
        float maxPhotonDistance,
        const Eigen::Vector3f & intersection) = 0;
    
    
    ///
    float gaussianWeight(float distSqrd, float radius);
    ///
    float gaussianWeightJensen(float distSqrd, float radius);
};


#endif /* PhotonMap_hpp */
