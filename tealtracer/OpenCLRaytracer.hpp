//
//  OpenCLRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OpenCLRaytracer_hpp
#define OpenCLRaytracer_hpp


#include <random>

#include "compute_engine.hpp"
#include "Raytracer.hpp"


class OpenCLRaytracer : public Raytracer {
public:
    
    ///
    OpenCLRaytracer();

    virtual void start();
    virtual void configure();
    
    /// Connects to computation device, creates buffers, and loads programs
    virtual void ocl_raytraceSetup();
    ///
    virtual void ocl_pushSceneData();
    
    ///
    void enqueRayTrace();
    virtual void ocl_raytraceRays() = 0;

protected:

    /// Tests the usage on "ComputeEngine" following the example given at the
    /// following web address:
    ///     https://developer.apple.com/library/mac/samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html
    ComputeEngine computeEngine;
    
    bool useGPU;
    unsigned int numSpheres, numPlanes, numLights;

};

#endif /* OpenCLRaytracer_hpp */
