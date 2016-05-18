//
//  OCLPhotonHashGridRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OCLPhotonHashGridRaytracer.hpp"

#include "TSLogger.hpp"
#include "stl_extensions.hpp"

#include "CLPovrayElementData.hpp"


///
OCLPhotonHashGridRaytracer::OCLPhotonHashGridRaytracer() : OpenCLRaytracer() {
    photonHashmap = std::shared_ptr<PhotonHashmap>(new PhotonHashmap());
}

///
void
OCLPhotonHashGridRaytracer::configure() {

    OpenCLRaytracer::configure();

    photonHashmap->cellsize = config.hashmapCellsize;
    photonHashmap->spacing = config.hashmapSpacing;
    photonHashmap->setDimensions(config.hashmapGridStart, config.hashmapGridEnd);
}

///
void OCLPhotonHashGridRaytracer::start() {

    configure();

    jobPool.emplaceJob(JobPool::WorkItem("[GPU] setup ray trace", [=](){
        ocl_raytraceSetup();
        double t0 = glfwGetTime();
        this->ocl_buildPhotonMap();
        double tf = glfwGetTime();
        TSLoggerLog(std::cout, "Done mapping photons: ", tf - t0);
        
    }, [=]() {
        this->enqueRayTrace();
    }));
}

///
void
OCLPhotonHashGridRaytracer::ocl_raytraceSetup() {
    
    OpenCLRaytracer::ocl_raytraceSetup();
    
    computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
    computeEngine.createKernel("raytrace_prog", "raytrace_one_ray_hashgrid");
    
    /// Photon mapping kernels
    computeEngine.createKernel("raytrace_prog", "emit_photon");
    computeEngine.createKernel("raytrace_prog", "photonmap_mapPhotonToGrid");
    computeEngine.createKernel("raytrace_prog", "photonmap_initGridFirstPhoton");
    computeEngine.createKernel("raytrace_prog", "photonmap_computeGridFirstPhoton");
    
    auto camera = config.scene->camera();
    
    //////
    /// Photon map
    //////
    
    if (config.raysPerLight > 0) {
        const int kNumFloatsInOCLPhoton = CLPackedPhoton_kNumFloats;
        //! TODO: We might need to multiply the number of lights
        computeEngine.createBuffer("photon_data", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * kNumFloatsInOCLPhoton * config.raysPerLight);
        computeEngine.createBuffer("map_gridIndices", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * config.raysPerLight);
    }
    
    int mapGridDimensions = photonHashmap->xdim * photonHashmap->ydim * photonHashmap->zdim;
    if (mapGridDimensions > 0) {
        computeEngine.createBuffer("map_gridFirstPhotonIndices", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * mapGridDimensions);
    }
    
    int photonAccumBufferSize = config.numberOfPhotonsToGather * outputImage.width * outputImage.height;
    computeEngine.createBuffer("photon_index_array", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * photonAccumBufferSize);
}

///
void
OCLPhotonHashGridRaytracer::ocl_buildPhotonMap() {
    ocl_emitPhotons();
    ocl_sortPhotons();
    ocl_mapPhotonsToGrid();
    ocl_computeGridFirstIndices();
}

///
void
OCLPhotonHashGridRaytracer::ocl_emitPhotons() {
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

///
void
OCLPhotonHashGridRaytracer::ocl_sortPhotons() {

    double startTime = glfwGetTime();
    
    std::vector<CLPackedPhoton> photons(config.raysPerLight, CLPackedPhoton());
    computeEngine.readBuffer("photon_data", 0, 0, sizeof(CLPackedPhoton) * config.raysPerLight, &photons[0]);
    std::sort(photons.begin(), photons.end(), [&](const CLPackedPhoton & a, const CLPackedPhoton & b) {
        return photonHashmap->getCellIndexHash(Eigen::Vector3f(a.pos_x, a.pos_y, a.pos_z)) < photonHashmap->getCellIndexHash(Eigen::Vector3f(b.pos_x, b.pos_y, b.pos_z));
    });
    computeEngine.writeBuffer("photon_data", 0, 0, sizeof(CLPackedPhoton) * config.raysPerLight, &photons[0]);
    ///
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed sort time: ", endTime - startTime);
}

///
void
OCLPhotonHashGridRaytracer::ocl_mapPhotonsToGrid() {

    double startTime = glfwGetTime();

    computeEngine.setKernelArgs("photonmap_mapPhotonToGrid",
        (cl_int) photonHashmap->spacing,
        (cl_float) photonHashmap->xmin,
        (cl_float) photonHashmap->ymin,
        (cl_float) photonHashmap->zmin,
        (cl_float) photonHashmap->xmax,
        (cl_float) photonHashmap->ymax,
        (cl_float) photonHashmap->zmax,
        (cl_int) photonHashmap->xdim,
        (cl_int) photonHashmap->ydim,
        (cl_int) photonHashmap->zdim,
        (cl_float) photonHashmap->cellsize,
    
        computeEngine.getBuffer("photon_data"),
        (cl_int) config.raysPerLight, // num_photons

        computeEngine.getBuffer("map_gridIndices"),
        computeEngine.getBuffer("map_gridFirstPhotonIndices")
    );

    computeEngine.executeKernel("photonmap_mapPhotonToGrid", 0, config.raysPerLight);
    computeEngine.finish(0);
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed toGrid time: ", endTime - startTime);
}

///
void
OCLPhotonHashGridRaytracer::ocl_computeGridFirstIndices() {

    double startTime = glfwGetTime();

    computeEngine.setKernelArgs("photonmap_initGridFirstPhoton",
        (cl_int) photonHashmap->xdim,
        (cl_int) photonHashmap->ydim,
        (cl_int) photonHashmap->zdim,
        computeEngine.getBuffer("map_gridFirstPhotonIndices")
    );
    
    computeEngine.executeKernel("photonmap_initGridFirstPhoton", 0, photonHashmap->xdim * photonHashmap->ydim * photonHashmap->zdim);

    computeEngine.setKernelArgs("photonmap_computeGridFirstPhoton",
        computeEngine.getBuffer("photon_data"),
        (cl_int) config.raysPerLight,
        
        computeEngine.getBuffer("map_gridIndices"),
        computeEngine.getBuffer("map_gridFirstPhotonIndices")
    );

    computeEngine.executeKernel("photonmap_computeGridFirstPhoton", 0, config.raysPerLight);
    computeEngine.finish(0);
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed first index time: ", endTime - startTime);
}

///
void
OCLPhotonHashGridRaytracer::ocl_raytraceRays() {
    
    unsigned int imageWidth = outputImage.width;
    unsigned int imageHeight = outputImage.height;
    void * imageData = outputImage.dataPtr();
    
    unsigned int rayCount = imageWidth * imageHeight;
    
    auto camera = config.scene->camera();
    auto cameraData = CLPovrayCameraData(camera->data());

    computeEngine.setKernelArgs("raytrace_one_ray_hashgrid",
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
        
        computeEngine.getBuffer("photon_index_array"),
        (cl_int) config.numberOfPhotonsToGather,
        (cl_float) config.maxPhotonGatherDistance,
        
        (cl_int) photonHashmap->spacing,
        (cl_float) photonHashmap->xmin,
        (cl_float) photonHashmap->ymin,
        (cl_float) photonHashmap->zmin,
        (cl_float) photonHashmap->xmax,
        (cl_float) photonHashmap->ymax,
        (cl_float) photonHashmap->zmax,
        (cl_int) photonHashmap->xdim,
        (cl_int) photonHashmap->ydim,
        (cl_int) photonHashmap->zdim,
        (cl_float) photonHashmap->cellsize,
    
        computeEngine.getBuffer("photon_data"),
        (cl_int) config.raysPerLight, // num_photons

        computeEngine.getBuffer("map_gridIndices"),
        computeEngine.getBuffer("map_gridFirstPhotonIndices"),
       
       computeEngine.getBuffer("image_output"),
       (cl_uint) imageWidth,
       (cl_uint) imageHeight
    );
    
    computeEngine.executeKernel("raytrace_one_ray_hashgrid", 0, rayCount);
    computeEngine.finish(0);
    
    computeEngine.readImage("image_output", 0, 0, 0, 0, imageWidth, imageHeight, 1, 0, 0, imageData);
}
