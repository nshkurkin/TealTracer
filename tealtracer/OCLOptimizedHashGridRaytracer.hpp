//
//  OCLOptimizedHashGridRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OCLOptimizedHashGridRaytracer_hpp
#define OCLOptimizedHashGridRaytracer_hpp

#include <random>

#include "OpenCLRaytracer.hpp"
#include "PhotonHashmap.hpp"


class OCLOptimizedHashGridRaytracer : public OpenCLRaytracer {
public:
    
    ///
    OCLOptimizedHashGridRaytracer();

    virtual void start();
    virtual void configure();
    
    ///
    virtual void ocl_raytraceSetup();
    ///
    virtual void ocl_raytraceRays();

private:

    ///
    void ocl_buildPhotonMap();
    ///
    void ocl_emitPhotons();
    ///
    void ocl_sortPhotons();
    ///
    void ocl_mapPhotonsToGrid();
    ///
    void ocl_computeGridFirstIndices();


    ///
    std::shared_ptr<PhotonHashmap> photonHashmap;

};

#endif /* OCLOptimizedHashGridRaytracer_hpp */
