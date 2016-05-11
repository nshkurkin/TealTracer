//
//  HashGridPhotonMapRenderer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef HashGridPhotonMapRenderer_hpp
#define HashGridPhotonMapRenderer_hpp

#include <random>

#include "compute_engine.hpp"

#include "Raytracer.hpp"
#include "RaytracingConfig.hpp"
#include "PhotonHashmap.hpp"


class HashGridPhotonMapRenderer : public Raytracer {
public:
    
    ///
    HashGridPhotonMapRenderer();

    virtual void start();
    
    ///
    void ocl_raytraceSetup();
    ///
    void ocl_pushSceneData();
    
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
    void enqueRayTrace();
    /// Tests the usage on "ComputeEngine" following the example given at the
    /// following web address:
    ///     https://developer.apple.com/library/mac/samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html
    void ocl_raytraceRays();

private:

    ComputeEngine computeEngine;
    
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<float> distribution;
    
    std::shared_ptr<PhotonHashmap> photonHashmap;
    
    bool useGPU;
    unsigned int numSpheres, numPlanes, numLights;

};

#endif /* HashGridPhotonMapRenderer_hpp */
