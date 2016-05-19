//
//  OCLMonteCarloRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OCLMonteCarloRaytracer.hpp"

#include "CLPovrayElementData.hpp"

///
void
OCLMonteCarloRaytracer::ocl_raytraceSetup() {
    OpenCLRaytracer::ocl_raytraceSetup();

    computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
    computeEngine.createKernel("raytrace_prog", "raytrace_one_ray_direct");
}

///
void
OCLMonteCarloRaytracer::ocl_raytraceRays() {
    unsigned int imageWidth = outputImage.width;
    unsigned int imageHeight = outputImage.height;
    void * imageData = outputImage.dataPtr();
    
    unsigned int rayCount = imageWidth * imageHeight;
    
    auto camera = config.scene->camera();
    auto cameraData = CLPovrayCameraData(camera->data());

    computeEngine.setKernelArgs("raytrace_one_ray_direct",
        cameraData.location,
        cameraData.up,
        cameraData.right,
        cameraData.lookAt,
       
        (cl_uint) config.brdfType,
        
        computeEngine.getBuffer("spheres"),
        (cl_uint) numSpheres,
        
        computeEngine.getBuffer("planes"),
        (cl_uint) numPlanes,
        
        computeEngine.getBuffer("lights"),
        (cl_uint) numLights,
       
        computeEngine.getBuffer("image_output"),
        (cl_uint) imageWidth,
        (cl_uint) imageHeight
    );
    
    computeEngine.executeKernel("raytrace_one_ray_direct", activeDevice, std::vector<size_t> {(size_t) rayCount});
    computeEngine.finish(activeDevice);
    
    computeEngine.readImage("image_output", activeDevice, 0, 0, 0, imageWidth, imageHeight, 1, 0, 0, imageData);
}
