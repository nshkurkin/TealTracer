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
    /// Call this after building the spatial hash.
    ///
    /// NOTE: flux = totalEnergy/(float)numPhotons;
    virtual RGBf gatherPhotons(
        int maxNumPhotonsToGather,
        int intersectedGeomId,
        const Eigen::Vector3f & intersection,
        const Eigen::Vector3f & normal,
        float flux) = 0;
    
};


#endif /* PhotonMap_hpp */
