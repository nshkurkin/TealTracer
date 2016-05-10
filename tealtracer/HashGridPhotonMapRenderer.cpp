//
//  HashGridPhotonMapRenderer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "HashGridPhotonMapRenderer.hpp"

///
HashGridPhotonMapRenderer::HashGridPhotonMapRenderer() {
    FPSsaved = 0.0;
    realtimeSaved = 0.0;
    
    useGPU = false;
    config = RaytracingConfig();
    photonHashmap = std::shared_ptr<PhotonHashmap>(new PhotonHashmap());
    
    generator = std::mt19937(randomDevice());
    distribution = std::uniform_real_distribution<float>(0.0,1.0);
    
    jobPool = JobPool(1);
    framesRendered = 0;
    lastRayTraceTime = rayTraceElapsedTime = 0;
    FPSsaved = realtimeSaved = 0;
}

///
void HashGridPhotonMapRenderer::setupDrawingInWindow(TSWindow * window) {
    
    TSLoggerLog(std::cout, glGetString(GL_VERSION));

    /// OpenGL
    glClearColor(0.3, 0.3, 0.3, 1.0);
    glEnable(GLenum(GL_DEPTH_TEST));
    glDepthFunc(GLenum(GL_LESS));
    
    outputImage.setDimensions(config.renderOutputWidth, config.renderOutputHeight);
    void * imageDataPtr = outputImage.dataPtr();
    target.init(outputImage.width, outputImage.height, imageDataPtr);
    
    ///
    lastX = std::numeric_limits<float>::infinity();
    lastY = std::numeric_limits<float>::infinity();
}

void HashGridPhotonMapRenderer::start() {

    useGPU = config.computationDevice == RaytracingConfig::ComputationDevice::GPU;
    photonHashmap->cellsize = config.hashmapCellsize;
    photonHashmap->spacing = config.hashmapSpacing;
    photonHashmap->setDimensions(config.hashmapGridStart, config.hashmapGridEnd);

    jobPool.emplaceJob(JobPool::WorkItem("[GPU] setup ray trace", [=](){
        ocl_raytraceSetup();
    }, [=]() {
        this->jobPool.emplaceJob(JobPool::WorkItem("[GPU] build photon map", [=](){
            double t0 = glfwGetTime();
            this->ocl_buildPhotonMap();
            double tf = glfwGetTime();
            TSLoggerLog(std::cout, "Done mapping photons: ", tf - t0);
        }, [=](){
            this->enqueRayTrace();
        }));
    }));
}

///
void HashGridPhotonMapRenderer::drawInWindow(TSWindow * window) {
    
    /// Draw the scene
    glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    target.draw();
    ///
    
    jobPool.checkAndUpdateFinishedJobs();

    float FPS = 0;
    if (rayTraceElapsedTime > 0.0001) {
        FPS = 1.0 / rayTraceElapsedTime;
    }
    
    if (framesRendered % 5 == 0 || rayTraceElapsedTime > 0.33) {
        FPSsaved = std::floor(FPS * 100.0) / 100.0;
        realtimeSaved = std::floor(rayTraceElapsedTime * 10000.0) / 10000.0;
    }
    
    window->setTitle(make_string("GPU Ray Tracer (FPS: ", FPSsaved, ", t: ", realtimeSaved, " frames: ", framesRendered, ")"));
}

///
void HashGridPhotonMapRenderer::windowResize(TSWindow * window, int w, int h) {
    
}

///
void HashGridPhotonMapRenderer::framebufferResize(TSWindow * window, int w, int h) {
    
}

///
void HashGridPhotonMapRenderer::windowClose(TSWindow * window) {
    
}

///
void HashGridPhotonMapRenderer::keyDown(TSWindow * window, int key, int scancode, int mods) {
    float transform = 1.0;
    switch (key) {
        case GLFW_KEY_A:
            config.scene->camera()->orientedTransform(-transform, 0, 0);
            break;
        case GLFW_KEY_S:
            config.scene->camera()->orientedTransform(0, -transform, 0);
            break;
        case GLFW_KEY_D:
            config.scene->camera()->orientedTransform(transform, 0, 0);
            break;
        case GLFW_KEY_W:
            config.scene->camera()->orientedTransform(0, transform, 0);
            break;
        case GLFW_KEY_Q:
            config.scene->camera()->orientedTransform(0, 0, -transform);
            break;
        case GLFW_KEY_E:
            config.scene->camera()->orientedTransform(0, 0, transform);
            break;
        case GLFW_KEY_B:
            TSLoggerLog(std::cout, "Breakpoint!");
            break;
        default:
            break;
    }
}

///
void HashGridPhotonMapRenderer::keyUp(TSWindow * window, int key, int scancode, int mods) {

}

///
void HashGridPhotonMapRenderer::mouseUp(TSWindow * window, int button, int mods) {

}

///
void HashGridPhotonMapRenderer::mouseDown(TSWindow * window, int button, int mods) {

}

///
void HashGridPhotonMapRenderer::mouseMoved(TSWindow * window, double x, double y) {
    if (lastX != std::numeric_limits<float>::infinity()
     && lastY != std::numeric_limits<float>::infinity()
     && window->keyDown(GLFW_KEY_C)) {
        
        double transform = 0.1;
        double dx = x - lastX;
        double dy = y - lastY;
        
        config.scene->camera()->rotate(config.Up, -transform * dy, -transform * dx);
    }
    
    lastX = x;
    lastY = y;
}

///
void HashGridPhotonMapRenderer::mouseScroll(TSWindow * window, double dx, double dy) {

}

///
void HashGridPhotonMapRenderer::enqueRayTrace() {

    jobPool.emplaceJob(JobPool::WorkItem("[GPU] raytrace", [=](){
        auto startTime = glfwGetTime();
        this->ocl_raytraceRays();
        auto endTime = glfwGetTime();
        lastRayTraceTime = endTime - startTime;
    }, [=](){
        rayTraceElapsedTime = lastRayTraceTime;
        framesRendered++;
        this->target.outputTexture->setNeedsUpdate();
        this->enqueRayTrace();
    }));
}

///
void HashGridPhotonMapRenderer::ocl_raytraceSetup() {
    
    if (useGPU) {
        computeEngine.connect(ComputeEngine::DEVICE_TYPE_GPU, 2, false);
    }
    else {
        computeEngine.connect(ComputeEngine::DEVICE_TYPE_CPU, 1, false);
    }
    
    computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
    computeEngine.createKernel("raytrace_prog", "raytrace_one_ray");
    
    /// Photon mapping kernels
    computeEngine.createKernel("raytrace_prog", "emit_photon");
    computeEngine.createKernel("raytrace_prog", "photonmap_mapPhotonToGrid");
    computeEngine.createKernel("raytrace_prog", "photonmap_initGridFirstPhoton");
    computeEngine.createKernel("raytrace_prog", "photonmap_computeGridFirstPhoton");
    
    auto camera = config.scene->camera();
    
    ocl_pushSceneData();
    
    //////
    /// Photon map
    //////
    
    if (config.raysPerLight > 0) {
        const int kNumFloatsInOCLPhoton = 9;
        //! TODO: We might need to multiply the number of lights
        computeEngine.createBuffer("map_photon_data", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * kNumFloatsInOCLPhoton * config.raysPerLight);
        computeEngine.createBuffer("map_gridIndices", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * config.raysPerLight);
    }
    
    int mapGridDimensions = photonHashmap->xdim * photonHashmap->ydim * photonHashmap->zdim;
    if (mapGridDimensions > 0) {
        computeEngine.createBuffer("map_gridFirstPhotonIndices", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * mapGridDimensions);
    }
    
    int photonAccumBufferSize = config.numberOfPhotonsToGather * outputImage.width * outputImage.height;
    computeEngine.createBuffer("photon_index_array", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * photonAccumBufferSize);
    
    computeEngine.createImage2D("image_output", ComputeEngine::MemFlags::MEM_WRITE_ONLY, ComputeEngine::ChannelOrder::RGBA, ComputeEngine::ChannelType::UNORM_INT8, outputImage.width, outputImage.height);
}

///
void HashGridPhotonMapRenderer::ocl_buildPhotonMap() {
    ocl_emitPhotons();
    ocl_sortPhotons();
    ocl_mapPhotonsToGrid();
    ocl_computeGridFirstIndices();
}

///
void HashGridPhotonMapRenderer::ocl_emitPhotons() {
    double startTime = glfwGetTime();
    float luminosityPerPhoton = (((float) config.lumensPerLight) / (float) config.raysPerLight);
    float randFloat = distribution(generator);
    unsigned int randVal = (int) (100000.0f * randFloat);

    TSLoggerLog(std::cout, "Seeding GPU with value=", randVal, " float=", randFloat);
    computeEngine.setKernelArgs("emit_photon",
        (cl_uint) randVal,
        (cl_uint) config.brdfType,
        (cl_int) config.usePhotonMappingForDirectIllumination,
        
        computeEngine.getBuffer("spheres"),
        (cl_uint) numSpheres,
        computeEngine.getBuffer("planes"),
        (cl_uint) numPlanes,
        computeEngine.getBuffer("lights"),
        (cl_uint) numLights,
        
        (cl_float) luminosityPerPhoton,
        (cl_float) config.photonBounceProbability,
        (cl_float) config.photonBounceEnergyMultipler,
        
        computeEngine.getBuffer("map_photon_data"),
        (cl_int) config.raysPerLight
    );

    computeEngine.executeKernel("emit_photon", 0, config.raysPerLight);
    computeEngine.finish(0);
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed emit time: ", endTime - startTime);
}

///
void HashGridPhotonMapRenderer::ocl_sortPhotons() {

    double startTime = glfwGetTime();
    
    ///
    packed_struct PackedPhoton {
        cl_float pos_x, pos_y, pos_z;
        cl_float dir_x, dir_y, dir_z;
        cl_float ene_x, ene_y, ene_z;
    };
    
    std::vector<PackedPhoton> photons(config.raysPerLight, PackedPhoton());
    computeEngine.readBuffer("map_photon_data", 0, 0, sizeof(PackedPhoton) * config.raysPerLight, &photons[0]);
    std::sort(photons.begin(), photons.end(), [&](const PackedPhoton & a, const PackedPhoton & b) {
        return photonHashmap->getCellIndexHash(Eigen::Vector3f(a.pos_x, a.pos_y, a.pos_z)) < photonHashmap->getCellIndexHash(Eigen::Vector3f(b.pos_x, b.pos_y, b.pos_z));
    });
    computeEngine.writeBuffer("map_photon_data", 0, 0, sizeof(PackedPhoton) * config.raysPerLight, &photons[0]);
    ///
    
    double endTime = glfwGetTime();
    TSLoggerLog(std::cout, "elapsed sort time: ", endTime - startTime);
}

///
void HashGridPhotonMapRenderer::ocl_mapPhotonsToGrid() {

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
    
        computeEngine.getBuffer("map_photon_data"),
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
void HashGridPhotonMapRenderer::ocl_computeGridFirstIndices() {

    double startTime = glfwGetTime();

    computeEngine.setKernelArgs("photonmap_initGridFirstPhoton",
        (cl_int) photonHashmap->xdim,
        (cl_int) photonHashmap->ydim,
        (cl_int) photonHashmap->zdim,
        computeEngine.getBuffer("map_gridFirstPhotonIndices")
    );
    
    computeEngine.executeKernel("photonmap_initGridFirstPhoton", 0, photonHashmap->xdim * photonHashmap->ydim * photonHashmap->zdim);

    computeEngine.setKernelArgs("photonmap_computeGridFirstPhoton",
        computeEngine.getBuffer("map_photon_data"),
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
void HashGridPhotonMapRenderer::ocl_pushSceneData() {

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
    
        computeEngine.writeBuffer("spheres", 0, 0, sizeof(cl_float) * sphereData.size(), &sphereData[0]);
    }
    if (planes.size() > 0) {
         if (computeEngine.getBuffer("planes") == nullptr) {
            computeEngine.createBuffer("planes", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * planeData.size());
        }
    
        computeEngine.writeBuffer("planes", 0, 0, sizeof(cl_float) * planeData.size(), &planeData[0]);
    }
    if (lights.size() > 0) {
        if (computeEngine.getBuffer("lights") == nullptr) {
            computeEngine.createBuffer("lights", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * lightData.size());
        }
    
        computeEngine.writeBuffer("lights", 0, 0, sizeof(cl_float) * lightData.size(), &lightData[0]);
    }
}

///
void HashGridPhotonMapRenderer::ocl_raytraceRays() {
    
    unsigned int imageWidth = outputImage.width;
    unsigned int imageHeight = outputImage.height;
    void * imageData = outputImage.dataPtr();
    
    unsigned int rayCount = imageWidth * imageHeight;
    
    auto camera = config.scene->camera();
    auto cameraData = CLPovrayCameraData(camera->data());

    computeEngine.setKernelArg("raytrace_one_ray", 0, cameraData.location);
    computeEngine.setKernelArg("raytrace_one_ray", 1, cameraData.up);
    computeEngine.setKernelArg("raytrace_one_ray", 2, cameraData.right);
    computeEngine.setKernelArg("raytrace_one_ray", 3, cameraData.lookAt);

    computeEngine.setKernelArgs("raytrace_one_ray",
        cameraData.location,
        cameraData.up,
        cameraData.right,
        cameraData.lookAt,
       
        (cl_uint) config.brdfType,
        
        (cl_int) config.usePhotonMappingForDirectIllumination,
        
        (cl_int) config.directIlluminationEnabled,
        (cl_int) config.indirectIlluminationEnabled,
        (cl_int) config.shadowsEnabled,
        
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
    
        computeEngine.getBuffer("map_photon_data"),
        (cl_int) config.raysPerLight, // num_photons

        computeEngine.getBuffer("map_gridIndices"),
        computeEngine.getBuffer("map_gridFirstPhotonIndices"),
       
       computeEngine.getBuffer("image_output"),
       (cl_uint) imageWidth,
       (cl_uint) imageHeight
    );
    
    computeEngine.executeKernel("raytrace_one_ray", 0, rayCount);
    computeEngine.finish(0);
    
    computeEngine.readImage("image_output", 0, 0, 0, 0, imageWidth, imageHeight, 1, 0, 0, imageData);
    target.outputTexture->setNeedsUpdate();
}
