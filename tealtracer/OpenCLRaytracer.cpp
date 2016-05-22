//
//  OpenCLRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OpenCLRaytracer.hpp"

#include "TSLogger.hpp"
#include "stl_extensions.hpp"

#include "CLPovrayElementData.hpp"


///
OpenCLRaytracer::OpenCLRaytracer() : Raytracer() {
    FPSsaved = 0.0;
    realtimeSaved = 0.0;
    useGPU = false;
    
    numSpheres = numPlanes = numLights = 0;
    activeDevice = 0;
}

///
void
OpenCLRaytracer::configure() {
    useGPU = config.computationDevice == RaytracingConfig::ComputationDevice::GPU;
    if (useGPU) {
        activeDevice = 1;
    }
}

///
void
OpenCLRaytracer::start() {

    configure();

    jobPool.emplaceJob(JobPool::WorkItem("[GPU] setup ray trace", [=](){
        ocl_raytraceSetup();
    }, [=]() {
        this->enqueueRaytrace();
    }));
}

///
void
OpenCLRaytracer::enqueueRaytrace() {
    jobPool.emplaceJob(JobPool::WorkItem("[GPU] raytrace", [=](){
        auto startTime = glfwGetTime();
        this->ocl_raytraceRays();
        auto endTime = glfwGetTime();
        lastRayTraceTime = endTime - startTime;
    }, [=](){
        rayTraceElapsedTime = lastRayTraceTime;
        framesRendered++;
        this->target.outputTexture->setNeedsUpdate();
        this->enqueueRaytrace();
    }));
}

///
void
OpenCLRaytracer::ocl_raytraceSetup() {
    
    if (useGPU) {
        computeEngine.connect(ComputeEngine::DEVICE_TYPE_GPU, 2, false, false);
    }
    else {
        computeEngine.connect(ComputeEngine::DEVICE_TYPE_CPU, 1, false, false);
    }
    
    ocl_pushSceneData();
    
    computeEngine.createImage2D("image_output", ComputeEngine::MemFlags::MEM_WRITE_ONLY, ComputeEngine::ChannelOrder::RGBA, ComputeEngine::ChannelType::UNORM_INT8, outputImage.width, outputImage.height);
}

///
void
OpenCLRaytracer::ocl_pushSceneData() {

    auto spheres = config.scene->findElements<PovraySphere>();
    std::vector<cl_float> sphereData;
    for (auto itr = spheres.begin(); itr != spheres.end(); itr++) {
        CLPovraySphereData((*itr)->data()).writeOutData(sphereData);
    }
    
    auto planes = config.scene->findElements<PovrayPlane>();
    std::vector<cl_float> planeData;
    for (auto itr = planes.begin(); itr != planes.end(); itr++) {
        CLPovrayPlaneData((*itr)->data()).writeOutData(planeData);
    }
    
    auto lights = config.scene->findElements<PovrayLightSource>();
    std::vector<cl_float> lightData;
    for (auto itr = lights.begin(); itr != lights.end(); itr++) {
        CLPovrayLightSourceData((*itr)->data()).writeOutData(lightData);
    }

    numSpheres = (unsigned int) spheres.size();
    numPlanes = (unsigned int) planes.size();
    numLights = (unsigned int) lights.size();
    
    /// Fill the buffers
    if (spheres.size() > 0) {
        if (computeEngine.getBuffer("spheres") == nullptr) {
            computeEngine.createBuffer("spheres", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * sphereData.size());
        }
    
        computeEngine.writeBuffer("spheres", activeDevice, 0, sizeof(cl_float) * sphereData.size(), &sphereData[0]);
    }
    if (planes.size() > 0) {
         if (computeEngine.getBuffer("planes") == nullptr) {
            computeEngine.createBuffer("planes", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * planeData.size());
        }
    
        computeEngine.writeBuffer("planes", activeDevice, 0, sizeof(cl_float) * planeData.size(), &planeData[0]);
    }
    if (lights.size() > 0) {
        if (computeEngine.getBuffer("lights") == nullptr) {
            computeEngine.createBuffer("lights", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * lightData.size());
        }
    
        computeEngine.writeBuffer("lights", activeDevice, 0, sizeof(cl_float) * lightData.size(), &lightData[0]);
    }
}

