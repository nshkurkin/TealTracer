//
//  OCLOptimizedTiledPhotonRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/19/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OCLOptimizedTiledPhotonRaytracer.hpp"

///
OCLOptimizedTiledPhotonRaytracer::OCLOptimizedTiledPhotonRaytracer() : OpenCLRaytracer() {
    photonTiler = std::shared_ptr<PhotonTiler>(new PhotonTiler());
    photonEmissionSeed = generator.randUInt();
}

///
void
OCLOptimizedTiledPhotonRaytracer::start() {

    configure();

    jobPool.emplaceJob(JobPool::WorkItem("[GPU] setup ray trace", [=](){
        this->ocl_raytraceSetup();
        
        double t0 = glfwGetTime();
        this->ocl_emitPhotons();
        double tf = glfwGetTime();
        TSLoggerLog(std::cout, "Done emitting photons: ", tf - t0);
        
    }, [=]() {
        this->enqueueRaytrace();
    }));
}

///
void
OCLOptimizedTiledPhotonRaytracer::enqueueRaytrace() {
    cachedCameraData = config.scene->camera()->data();
    OpenCLRaytracer::enqueueRaytrace();
}

///
void
OCLOptimizedTiledPhotonRaytracer::PackedPlane::fromPlane(const PhotonTiler::Plane & plane) {
    plane_normal_x = plane.normal.x();
    plane_normal_y = plane.normal.y();
    plane_normal_z = plane.normal.z();
    plane_distance = plane.distance;
}

///
void
OCLOptimizedTiledPhotonRaytracer::PackedFrustum::fromFrustum(const PhotonTiler::Frustum & frustum) {
    for (int i = 0; i < 4; i++) {
        planes[i].fromPlane(frustum.planes[i]);
    }
}

///
void
OCLOptimizedTiledPhotonRaytracer::PackedTile::fromTile(const PhotonTiler::Tile & tile) {
    frustum.fromFrustum(tile.frustum);
}

///
void
OCLOptimizedTiledPhotonRaytracer::ocl_raytraceSetup() {

    OpenCLRaytracer::ocl_raytraceSetup();

    computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
    computeEngine.createKernel("raytrace_prog", "raytrace_one_ray_one_tile");
    
    /// Photon mapping kernels
    computeEngine.createKernel("raytrace_prog", "emit_photon");
    computeEngine.createKernel("raytrace_prog", "countPhotonsInTile");
    computeEngine.createKernel("raytrace_prog", "copyPhotonsIntoOneTile");

    computeEngine.createBuffer("photons", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(CLPackedPhoton) * config.raysPerLight);
    
    /// Now generate known tile buffers
    photonTiler->generateTiles(outputImage.width, outputImage.height, config.tile_width, config.tile_height, config.scene->camera()->location(), config.scene->camera()->basisVectors());
    tilePhotonCount = std::vector<cl_int> (photonTiler->tiles.size(), 0);
    
    computeEngine.createBuffer("tiles", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(PackedTile) * photonTiler->tiles.size());
    computeEngine.createBuffer("tilePhotonCount", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * photonTiler->tiles.size());
    computeEngine.createBuffer("nextPhotonIndex", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * photonTiler->tiles.size());
}

///
void
OCLOptimizedTiledPhotonRaytracer::ocl_emitPhotons() {
    double startTime = glfwGetTime();
    float luminosityPerPhoton = (((float) config.lumensPerLight) / (float) config.raysPerLight);

    computeEngine.setKernelArgs("emit_photon",
        (cl_uint) photonEmissionSeed,
        (cl_uint) config.brdfType,
        (cl_int) true, // dummy argument
        
        computeEngine.getBuffer("spheres"),
        (cl_uint) numSpheres,
        computeEngine.getBuffer("planes"),
        (cl_uint) numPlanes,
        computeEngine.getBuffer("lights"),
        (cl_uint) numLights,
        
        (cl_float) luminosityPerPhoton,
        (cl_float) config.photonBounceProbability,
        (cl_float) config.photonBounceEnergyMultipler,
        
        computeEngine.getBuffer("photons"),
        (cl_int) config.raysPerLight
    );

    computeEngine.executeKernel("emit_photon", activeDevice, std::vector<size_t> {(size_t) config.raysPerLight});
    computeEngine.finish(activeDevice);
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed emit time: ", endTime - startTime);
}

///
void
OCLOptimizedTiledPhotonRaytracer::ocl_buildAndFillTiles() {
    
    std::vector<cl_int> allZeros(photonTiler->tiles.size(), 0);
    /// Generate tiles
    photonTiler->generateTiles(outputImage.width, outputImage.height, config.tile_width, config.tile_height,cachedCameraData.getLocation(), cachedCameraData.basisVectors());
    
    std::vector<cl_float> tileData(photonTiler->tiles.size() * sizeof(PackedTile)/sizeof(cl_float), 0.0f);
    struct PackedTile tile;
    for (size_t tileItr = 0; tileItr < photonTiler->tiles.size(); tileItr++) {
        tile.fromTile(photonTiler->tiles[tileItr]);
        memcpy(&tileData[tileItr * sizeof(PackedTile)/sizeof(cl_float)], &tile, sizeof(PackedTile));
    }
    computeEngine.writeBuffer("tiles", activeDevice, 0, sizeof(PackedTile) * photonTiler->tiles.size(), &tileData[0]);
    computeEngine.writeBuffer("tilePhotonCount", activeDevice, 0, sizeof(cl_int) * photonTiler->tiles.size(), &allZeros[0]);
    
    /// Counting pass
    computeEngine.setKernelArgs("countPhotonsInTile",
        computeEngine.getBuffer("photons"),
        (cl_int) config.raysPerLight,

        computeEngine.getBuffer("tiles"),
        (cl_int) photonTiler->tiles.size(),

        (cl_float) config.tile_photonEffectRadius,
        
        computeEngine.getBuffer("tilePhotonCount")
    );
    
    computeEngine.executeKernel("countPhotonsInTile", activeDevice, std::vector<size_t> {(size_t) config.raysPerLight});
    
    /// Allocation
    computeEngine.readBuffer("tilePhotonCount", activeDevice, 0, sizeof(cl_int) * photonTiler->tiles.size(), &tilePhotonCount[0]);
    
    for (int tileItr = 0; tileItr < photonTiler->tiles.size(); tileItr++) {
        computeEngine.createBuffer(tilePhotonBufferName(tileItr).c_str(), ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(CLPackedPhoton) * std::max<int>(tilePhotonCount[tileItr], 1));
    }
    
    /// Copy pass
    computeEngine.writeBuffer("nextPhotonIndex", activeDevice, 0, sizeof(cl_int) * allZeros.size(), &allZeros[0]);
    for (int tileItr = 0; tileItr < photonTiler->tiles.size(); tileItr++) {
        
        std::string bufferName = tilePhotonBufferName(tileItr);
        assert(computeEngine.getBuffer(tilePhotonBufferName(tileItr).c_str()) != NULL);
        computeEngine.setKernelArgs("copyPhotonsIntoOneTile",
            computeEngine.getBuffer("photons"),
            (cl_int) config.raysPerLight,
            
            (cl_float) config.tile_photonEffectRadius,
            
            (cl_int) tileItr,
            computeEngine.getBuffer("tiles"),
            computeEngine.getBuffer(tilePhotonBufferName(tileItr).c_str()),
            computeEngine.getBuffer("nextPhotonIndex")
        );
        
        computeEngine.executeKernel("copyPhotonsIntoOneTile", activeDevice, std::vector<size_t> {(size_t) config.raysPerLight});
    }
    
    computeEngine.finish(activeDevice);
}

///
void
OCLOptimizedTiledPhotonRaytracer::ocl_raytraceRays() {
    
//    ocl_emitPhotons();//
    
    double fillT0 = glfwGetTime();
    ocl_buildAndFillTiles();
    double fillTf = glfwGetTime();
    double tileT0 = glfwGetTime();

    for (int tileItr = 0; tileItr < photonTiler->tiles.size(); tileItr++) {
        int tileX = tileItr % (outputImage.width/config.tile_width);
        int tileY = tileItr / (outputImage.width/config.tile_width);
    
        computeEngine.setKernelArgs("raytrace_one_ray_one_tile",
            cachedCameraData.location,
            cachedCameraData.up,
            cachedCameraData.right,
            cachedCameraData.lookAt,
           
            (cl_uint) config.brdfType,
            
            computeEngine.getBuffer("spheres"),
            (cl_uint) numSpheres,
            
            computeEngine.getBuffer("planes"),
            (cl_uint) numPlanes,
            
            computeEngine.getBuffer("lights"),
            (cl_uint) numLights,
            
            ///
            (cl_int) tileX,
            (cl_int) tileY,
            (cl_int) config.tile_width,
            (cl_int) config.tile_height,
            (cl_float) config.tile_photonEffectRadius,
            (cl_float) config.tile_photonSampleRate,
            computeEngine.getBuffer(tilePhotonBufferName(tileItr).c_str()),
            (cl_int) tilePhotonCount[tileItr],
            ///
           
            computeEngine.getBuffer("image_output"),
            (cl_uint) outputImage.width,
            (cl_uint) outputImage.height
        );
    
        computeEngine.executeKernel("raytrace_one_ray_one_tile", activeDevice, std::vector<size_t> {(size_t) config.tile_width * config.tile_height});
    }
    
    computeEngine.finish(activeDevice);
    
    double tileTf = glfwGetTime();
    TSLoggerLog(std::cout, "build and fill time: ", fillTf - fillT0);
    TSLoggerLog(std::cout, "render time: ", tileTf - tileT0);
    
    computeEngine.readImage("image_output", activeDevice, 0, 0, 0, outputImage.width, outputImage.height, 1, 0, 0, outputImage.dataPtr());
}
