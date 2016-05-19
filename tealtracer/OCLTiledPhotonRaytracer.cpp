//
//  OCLTiledPhotonRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OCLTiledPhotonRaytracer.hpp"

///
OCLTiledPhotonRaytracer::OCLTiledPhotonRaytracer() : OpenCLRaytracer() {
    photonTiler = std::shared_ptr<PhotonTiler>(new PhotonTiler());
}

///
void
OCLTiledPhotonRaytracer::start() {

    configure();

    jobPool.emplaceJob(JobPool::WorkItem("[GPU] setup ray trace", [=](){
        this->ocl_raytraceSetup();
        
        double t0 = glfwGetTime();
        this->ocl_emitPhotons();
        double tf = glfwGetTime();
        TSLoggerLog(std::cout, "Done emitting photons: ", tf - t0);
        
    }, [=]() {
        this->enqueRayTrace();
    }));
}

///
void
OCLTiledPhotonRaytracer::PackedPlane::fromPlane(const PhotonTiler::Plane & plane) {
    plane_normal_x = plane.normal.x();
    plane_normal_y = plane.normal.y();
    plane_normal_z = plane.normal.z();
    plane_distance = plane.distance;
}

///
void
OCLTiledPhotonRaytracer::PackedFrustum::fromFrustum(const PhotonTiler::Frustum & frustum) {
    for (int i = 0; i < 4; i++) {
        planes[i].fromPlane(frustum.planes[i]);
    }
}

///
void
OCLTiledPhotonRaytracer::PackedTile::fromTile(const PhotonTiler::Tile & tile) {
    frustum.fromFrustum(tile.frustum);
}

///
void
OCLTiledPhotonRaytracer::ocl_raytraceSetup() {

    OpenCLRaytracer::ocl_raytraceSetup();

    computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
    computeEngine.createKernel("raytrace_prog", "raytrace_one_ray_tiled");
    
    /// Photon mapping kernels
    computeEngine.createKernel("raytrace_prog", "emit_photon");
    computeEngine.createKernel("raytrace_prog", "countPhotonsInTile");
    computeEngine.createKernel("raytrace_prog", "copyPhotonsIntoTile");

    computeEngine.createBuffer("photons", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(CLPackedPhoton) * config.raysPerLight);

//        size_t packedPhotonSize = sizeof(CLPackedPhoton);
//        size_t clFloatSize = sizeof(cl_float);
//        size_t numFloatsInPhoton = packedPhotonSize / clFloatSize;
//        size_t reportedNumberOfFloatsInPhoton = CLPackedPhoton_kNumFloats;
//    
//        size_t packedTileSize = sizeof(PackedTile);
//        size_t numFloatsInTile = sizeof(PackedTile) / sizeof(cl_float);
//        size_t expectedNumerOfFloats = 4 * 4;
    
    /// Now generate known tile buffers
    photonTiler->generateTiles(outputImage.width, outputImage.height, config.tile_width, config.tile_height, config.scene->camera()->location(), config.scene->camera()->basisVectors());
    
    computeEngine.createBuffer("tiles", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(PackedTile) * photonTiler->tiles.size());
    computeEngine.createBuffer("tilePhotonCount", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * photonTiler->tiles.size());
    computeEngine.createBuffer("nextPhotonIndex", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * photonTiler->tiles.size());
    /// Cannot create "tilePhotons" yet
    computeEngine.createBuffer("tilePhotonStarts", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_int) * photonTiler->tiles.size());
}

///
void
OCLTiledPhotonRaytracer::ocl_emitPhotons() {
    double startTime = glfwGetTime();
    float luminosityPerPhoton = (((float) config.lumensPerLight) / (float) config.raysPerLight);

    computeEngine.setKernelArgs("emit_photon",
        (cl_uint) generator.randUInt(),
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

    computeEngine.executeKernel("emit_photon", 0, std::vector<size_t> {(size_t) config.raysPerLight});
    computeEngine.finish(0);
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed emit time: ", endTime - startTime);
}

///
void
OCLTiledPhotonRaytracer::ocl_buildAndFillTiles() {
    
    std::vector<cl_int> allZeros(photonTiler->tiles.size(), 0);
    /// Generate tiles
    photonTiler->generateTiles(outputImage.width, outputImage.height, config.tile_width, config.tile_height, config.scene->camera()->location(), config.scene->camera()->basisVectors());
    
    std::vector<cl_float> tileData(photonTiler->tiles.size() * sizeof(PackedTile)/sizeof(cl_float), 0.0f);
    struct PackedTile tile;
    for (size_t tileItr = 0; tileItr < photonTiler->tiles.size(); tileItr++) {
        tile.fromTile(photonTiler->tiles[tileItr]);
        memcpy(&tileData[tileItr * sizeof(PackedTile)/sizeof(cl_float)], &tile, sizeof(PackedTile));
    }
    computeEngine.writeBuffer("tiles", 0, 0, sizeof(PackedTile) * photonTiler->tiles.size(), &tileData[0]);
    computeEngine.writeBuffer("tilePhotonCount", 0, 0, sizeof(cl_int) * photonTiler->tiles.size(), &allZeros[0]);
    
    /// Counting pass
    computeEngine.setKernelArgs("countPhotonsInTile",
        computeEngine.getBuffer("photons"),
        (cl_int) config.raysPerLight,

        computeEngine.getBuffer("tiles"),
        (cl_int) photonTiler->tiles.size(),

        (cl_float) config.tile_photonEffectRadius,
        
        computeEngine.getBuffer("tilePhotonCount")
    );
    
    computeEngine.executeKernel("countPhotonsInTile", 0, std::vector<size_t> {(size_t) config.raysPerLight});
    
    /// Allocation pass
    /// Now count the photons and allocate enough space for the photons
    std::vector<cl_int> tilePhotonStarts(photonTiler->tiles.size(), 0);
    std::vector<cl_int> tilePhotonCount(photonTiler->tiles.size(), 0);
    
    computeEngine.readBuffer("tilePhotonCount", 0, 0, sizeof(cl_int) * photonTiler->tiles.size(), &tilePhotonCount[0]);
    int accumStart = 0;
    for (size_t tileItr = 0; tileItr < photonTiler->tiles.size(); tileItr++) {
        tilePhotonStarts[tileItr] = accumStart;
        accumStart += tilePhotonCount[tileItr];
    }
    computeEngine.writeBuffer("tilePhotonStarts", 0, 0, sizeof(cl_int) * photonTiler->tiles.size(), &tilePhotonStarts[0]);
    computeEngine.createBuffer("tilePhotons", ComputeEngine::MemFlags::MEM_READ_ONLY, accumStart * sizeof(CLPackedPhoton));
    
    /// Copy Pass
    computeEngine.writeBuffer("nextPhotonIndex", 0, 0, sizeof(cl_int) * allZeros.size(), &allZeros[0]);
    computeEngine.setKernelArgs("copyPhotonsIntoTile",
        computeEngine.getBuffer("photons"),
        (cl_int) config.raysPerLight,

        computeEngine.getBuffer("tiles"),
        (cl_int) photonTiler->tiles.size(),

        (cl_float) config.tile_photonEffectRadius,
        computeEngine.getBuffer("tilePhotonCount"),

        computeEngine.getBuffer("nextPhotonIndex"),
        computeEngine.getBuffer("tilePhotons"),
        computeEngine.getBuffer("tilePhotonStarts")
    );
    
    computeEngine.executeKernel("copyPhotonsIntoTile", 0, std::vector<size_t> {(size_t) config.raysPerLight});
//    computeEngine.finish(0);
}

///
void
OCLTiledPhotonRaytracer::ocl_raytraceRays() {
    
    double fillT0 = glfwGetTime();
    
    ocl_buildAndFillTiles();
    
    double fillTf = glfwGetTime();
    double tileT0 = glfwGetTime();
    
    auto camera = config.scene->camera();
    auto cameraData = CLPovrayCameraData(camera->data());

    computeEngine.setKernelArgs("raytrace_one_ray_tiled",
        cameraData.location,
        cameraData.up,
        cameraData.right,
        cameraData.lookAt,
       
        (cl_uint) config.brdfType,
        (cl_uint) generator.randUInt(),
        
        computeEngine.getBuffer("spheres"),
        (cl_uint) numSpheres,
        
        computeEngine.getBuffer("planes"),
        (cl_uint) numPlanes,
        
        computeEngine.getBuffer("lights"),
        (cl_uint) numLights,
        
        ///
        (cl_int) config.tile_width,
        (cl_int) config.tile_height,
        (cl_float) config.tile_photonEffectRadius,
        (cl_float) config.tile_photonSampleRate,
        computeEngine.getBuffer("tilePhotons"),
        computeEngine.getBuffer("tilePhotonCount"),
        computeEngine.getBuffer("tilePhotonStarts"),
        ///
       
        computeEngine.getBuffer("image_output"),
        (cl_uint) outputImage.width,
        (cl_uint) outputImage.height
    );
    
    computeEngine.executeKernel("raytrace_one_ray_tiled", 0, {(size_t) outputImage.width, (size_t) outputImage.height});
//    computeEngine.finish(0);
    
    double tileTf = glfwGetTime();
    TSLoggerLog(std::cout, "build and fill time: ", fillTf - fillT0);
    TSLoggerLog(std::cout, "render time: ", tileTf - tileT0);
    
    computeEngine.readImage("image_output", 0, 0, 0, 0, outputImage.width, outputImage.height, 1, 0, 0, outputImage.dataPtr());
}
