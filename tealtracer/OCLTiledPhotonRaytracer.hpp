//
//  OCLTiledPhotonRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OCLTiledPhotonRaytracer_hpp
#define OCLTiledPhotonRaytracer_hpp

#include "OpenCLRaytracer.hpp"
#include "CLPovrayElementData.hpp"

class OCLTiledPhotonRaytracer : public OpenCLRaytracer {
public:

    
    virtual void ocl_raytraceSetup() {
    
        OpenCLRaytracer::ocl_raytraceSetup();
    
        computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
//        computeEngine.createKernel("raytrace_prog", "raytrace_one_ray");
        
        /// Photon mapping kernels
        computeEngine.createKernel("raytrace_prog", "emit_photon");
    
        if (config.raysPerLight > 0) {
            const int kNumFloatsInOCLPhoton = CLPackedPhoton_kNumFloats;
            computeEngine.createBuffer("photon_data", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * kNumFloatsInOCLPhoton * config.raysPerLight);
        }
    }

    ///
    void ocl_emitPhotons() {
        double startTime = glfwGetTime();
        float luminosityPerPhoton = (((float) config.lumensPerLight) / (float) config.raysPerLight);
        float randFloat = generator.randFloat();
        unsigned int randVal = (int) (100000.0f * randFloat);

        computeEngine.setKernelArgs("emit_photon",
            (cl_uint) randVal,
            (cl_uint) config.brdfType,
            
            computeEngine.getBuffer("spheres"),
            (cl_uint) numSpheres,
            computeEngine.getBuffer("planes"),
            (cl_uint) numPlanes,
            computeEngine.getBuffer("lights"),
            (cl_uint) numLights,
            
            (cl_float) luminosityPerPhoton,
            (cl_float) config.photonBounceProbability,
            (cl_float) config.photonBounceEnergyMultipler,
            
            computeEngine.getBuffer("photon_data"),
            (cl_int) config.raysPerLight
        );

        computeEngine.executeKernel("emit_photon", 0, config.raysPerLight);
        computeEngine.finish(0);
        
        double endTime = glfwGetTime();
        TSLoggerLog(std::cout, "elapsed emit time: ", endTime - startTime);
    }

};

#endif /* OCLTiledPhotonRaytracer_hpp */
