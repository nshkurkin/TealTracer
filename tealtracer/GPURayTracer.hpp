//
//  GPURayTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef GPURayTracer_hpp
#define GPURayTracer_hpp

#include <random>

#include "TSWindow.hpp"
#include "gl_include.h"
#include "opengl_errors.hpp"
#include "compute_engine.hpp"
#include "stl_extensions.hpp"

#include "OpenGLShaders.hpp"
#include "PovrayScene.hpp"
#include "TSLogger.hpp"
#include "TextureRenderTarget.hpp"
#include "Image.hpp"
#include "CLPovrayElementData.hpp"
#include "JobPool.hpp"
#include "PhotonHashmap.hpp"

class GPURayTracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:

    ///
    /// Rendering pipeline setup:
    ///     1) Have an OpenGL texture to display on screen, connect it to GPU
    ///     2) Perform raycasts on the GPU and fill the texture with content
    ///     3) Display the texture
    ///
    
    ///
    /// Some ideas:
    ///     *) Use an OpenGL texture reference instead of a local copy buffer
    ///             Note: this will likely only save about 2-3 ms per frame
    ///     *) Build in a solid system for camera controls.
    ///     *) Port photon mapping from Swift project
    ///     *) Implement a photon mapping kernel
    
    ComputeEngine computeEngine;
    TextureRenderTarget target;
    
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution;
    
    Image outputImage;
    int renderOutputWidth;
    int renderOutputHeight;

    enum SupportedBRDF {
        BlinnPhong = 0, // https://en.wikipedia.org/wiki/Blinn–Phong_shading_model
        OrenNayar = 1 // https://en.wikipedia.org/wiki/Oren–Nayar_reflectance_model
    };
    
    SupportedBRDF brdfType;
    
    int numberOfPhotonsToGather;
    int raysPerLight;
    int lumensPerLight;
    
    float photonBounceProbability;
    float photonBounceEnergyMultipler;
    
    std::shared_ptr<PhotonHashmap> photonHashmap;
    
    
    static const Eigen::Vector3f Up;
    static const Eigen::Vector3f Forward;
    static const Eigen::Vector3f Right;

    ///
    virtual void setupDrawingInWindow(TSWindow * window) {
        
        TSLoggerLog(std::cout, glGetString(GL_VERSION));
    
        /// OpenGL
        glClearColor(0.3, 0.3, 0.3, 1.0);
        glEnable(GLenum(GL_DEPTH_TEST));
        glDepthFunc(GLenum(GL_LESS));
        
        outputImage.setDimensions(renderOutputWidth, renderOutputHeight);
        target.init(outputImage.width, outputImage.height, outputImage.dataPtr());
        
        FPSsaved = 0.0;
        realtimeSaved = 0.0;
        brdfType = BlinnPhong;
        
        photonHashmap = std::shared_ptr<PhotonHashmap>(new PhotonHashmap());
        photonHashmap->setDimensions(Eigen::Vector3f(-20,-20,-20), Eigen::Vector3f(20,20,20));
        
        distribution = std::uniform_real_distribution<float>(0.0,1.0);
        
        ///
        lastX = std::numeric_limits<float>::infinity();
        lastY = std::numeric_limits<float>::infinity();
    }

    float FPSsaved, realtimeSaved;

    void start() {
        ocl_raytraceSetup();
        
        ocl_buildPhotonMap();
        enqueRayTrace();
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        target.draw();
        
        jobPool.checkAndUpdateFinishedJobs();
    
        float FPS = 0;
        if (rayTraceElapsedTime > 0.001) {
            FPS = 1.0 / rayTraceElapsedTime;
        }
        
        if (framesRendered % 10 == 0) {
            FPSsaved = std::floor(FPS * 100.0) / 100.0;
            realtimeSaved = std::floor(rayTraceElapsedTime * 10000.0) / 10000.0;
        }
        
        window->setTitle(make_string("GPU Ray Tracer (FPS: ", FPSsaved, ", t: ", realtimeSaved, " frames: ", framesRendered, ")"));
    }
    
    ///
    virtual void windowResize(TSWindow * window, int w, int h) {
        
    }
    
    ///
    virtual void framebufferResize(TSWindow * window, int w, int h) {
        
    }

    ///
    virtual void windowClose(TSWindow * window) {
        
    }
    
    ///
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods) {
        float transform = 1.0;
        switch (key) {
            case GLFW_KEY_A:
                scene_->camera()->orientedTransform(-transform, 0, 0);
                break;
            case GLFW_KEY_S:
                scene_->camera()->orientedTransform(0, -transform, 0);
                break;
            case GLFW_KEY_D:
                scene_->camera()->orientedTransform(transform, 0, 0);
                break;
            case GLFW_KEY_W:
                scene_->camera()->orientedTransform(0, transform, 0);
                break;
            case GLFW_KEY_Q:
                scene_->camera()->orientedTransform(0, 0, -transform);
                break;
            case GLFW_KEY_E:
                scene_->camera()->orientedTransform(0, 0, transform);
                break;
            default:
                break;
        }
    }
    
    ///
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods) {
    
    }
    
    ///
    virtual void mouseUp(TSWindow * window, int button, int mods) {
    
    }
    
    ///
    virtual void mouseDown(TSWindow * window, int button, int mods) {
    
    }
    
    double lastX, lastY;
    
    ///
    virtual void mouseMoved(TSWindow * window, double x, double y) {
        if (lastX != std::numeric_limits<float>::infinity()
         && lastY != std::numeric_limits<float>::infinity()
         && window->keyDown(GLFW_KEY_C)) {
            
            double transform = 0.1;
            double dx = x - lastX;
            double dy = y - lastY;
            
            scene_->camera()->rotate(Up, -transform * dy, -transform * dx);
        }
        
        lastX = x;
        lastY = y;
    }
    
    ///
    virtual void mouseScroll(TSWindow * window, double dx, double dy) {
    
    }
    
    
    JobPool jobPool;
    int framesRendered;
    double lastRayTraceTime, rayTraceElapsedTime;
    
    ///
    void enqueRayTrace() {
        jobPool.emplaceJob([=](){
            auto startTime = glfwGetTime();
            this->ocl_raytraceRays();
            auto endTime = glfwGetTime();
            lastRayTraceTime = endTime - startTime;
        }, [=](){
            rayTraceElapsedTime = lastRayTraceTime;
            framesRendered++;
            this->target.outputTexture->setNeedsUpdate();
            this->enqueRayTrace();
        });
    }

    bool useGPU;
    unsigned int numSpheres, numPlanes, numLights;
    
    ///
    void ocl_raytraceSetup() {
        
        if (useGPU) {
            computeEngine.connect(ComputeEngine::DEVICE_TYPE_GPU, 2, true);
        }
        else {
            computeEngine.connect(ComputeEngine::DEVICE_TYPE_CPU, 4, false);
        }

        size_t imageDataSize = outputImage.dataSize();
        
        computeEngine.createProgramFromFile("raytrace_prog", "raytrace.cl");
        computeEngine.createKernel("raytrace_prog", "raytrace_one_ray");
        
        /// Photon mapping kernels
        computeEngine.createKernel("raytrace_prog", "emit_photon");
        computeEngine.createKernel("raytrace_prog", "photonmap_sortPhotons");
        computeEngine.createKernel("raytrace_prog", "photonmap_mapPhotonToGrid");
        computeEngine.createKernel("raytrace_prog", "photonmap_initGridFirstPhoton");
        computeEngine.createKernel("raytrace_prog", "photonmap_computeGridFirstPhoton");
        
        auto camera = scene_->camera();
        
        ocl_pushSceneData();
        
        computeEngine.createBuffer("imageOutput", ComputeEngine::MemFlags::MEM_WRITE_ONLY, imageDataSize);
        
        //////
        /// Photon map
        //////
        
        if (raysPerLight > 0) {
            const int kNumFloatsInOCLPhoton = 9;
            //! TODO: We might need to multiply the number of lights
            computeEngine.createBuffer("map_photon_data", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * kNumFloatsInOCLPhoton * raysPerLight);
            computeEngine.createBuffer("map_gridIndices", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * raysPerLight);
        }
        
        int mapGridDimensions = photonHashmap->xdim * photonHashmap->ydim * photonHashmap->zdim;
        if (mapGridDimensions > 0) {
            computeEngine.createBuffer("map_gridFirstPhotonIndices", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * mapGridDimensions);
        }
        
        int photonAccumBufferSize = numberOfPhotonsToGather * outputImage.width * outputImage.height;
        computeEngine.createBuffer("photon_index_array", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_int) * photonAccumBufferSize);
        computeEngine.createBuffer("photon_distance_array", ComputeEngine::MemFlags::MEM_READ_WRITE, sizeof(cl_float) * photonAccumBufferSize);
    }
    
    ///
    void ocl_buildPhotonMap() {
        ocl_emitPhotons();
        ocl_sortPhotons();
        ocl_mapPhotonsToGrid();
        ocl_computeGridFirstIndices();
    }
    
    ///
    void ocl_emitPhotons() {
        double startTime = glfwGetTime();
        float luminosityPerPhoton = (((float) lumensPerLight) / (float) raysPerLight);
    
        computeEngine.setKernelArgs("emit_photon",
            (cl_uint) (100000.0f * distribution(generator)),
            (cl_uint) brdfType,
            
            computeEngine.getBuffer("spheres"),
            (cl_uint) numSpheres,
            computeEngine.getBuffer("planes"),
            (cl_uint) numPlanes,
            computeEngine.getBuffer("lights"),
            (cl_uint) numLights,
            
            (cl_float) luminosityPerPhoton,
            (cl_float) photonBounceProbability,
            (cl_float) photonBounceEnergyMultipler,
            
            computeEngine.getBuffer("map_photon_data"),
            (cl_int) raysPerLight
        );

        size_t globalCount = raysPerLight;
        size_t localCount = localCountForGlobalCount("emit_photon", globalCount);

        computeEngine.executeKernel("emit_photon", 0, &globalCount, &localCount, 1);
        
        double endTime = glfwGetTime();
        TSLoggerLog(std::cout, "elapsed emit time: ", endTime - startTime);
    }
    
    ///
    size_t localCountForGlobalCount(const std::string & kernel, size_t count) {
        size_t local = computeEngine.getEstimatedWorkGroupSize(kernel.c_str());
        while (count % local != 0) {
            local--;
        }
        return local;
    }
    
    ///
    void ocl_sortPhotons() {
        int numPhotons = raysPerLight;
        size_t globalCount = raysPerLight / 2;
        size_t localCount = localCountForGlobalCount("photonmap_sortPhotons", globalCount);
        
        double startTime = glfwGetTime();
        
        for (int itr = 0; itr < numPhotons; itr++) {
            int parity = itr % 2;
        
            computeEngine.setKernelArgs("photonmap_sortPhotons",
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
                (cl_int) numPhotons,
                
                (cl_int) parity
            );
            
            computeEngine.executeKernel("photonmap_sortPhotons", 0, &globalCount, &localCount, 1);
        }
        
        double endTime = glfwGetTime();
        TSLoggerLog(std::cout, "elapsed sort time: ", endTime - startTime);
    }
    
    ///
    void ocl_mapPhotonsToGrid() {
    
        ///
//        std::vector<cl_float> photonData(raysPerLight * 9, -1000.0f);
//        computeEngine.readBuffer("map_photon_data", 0, 0, sizeof(cl_float) * raysPerLight * 9, &photonData[0]);
        ///
    
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
            (cl_int) raysPerLight, // num_photons

            computeEngine.getBuffer("map_gridIndices"),
            computeEngine.getBuffer("map_gridFirstPhotonIndices")
        );
    
        size_t globalCount = raysPerLight;
        size_t localCount = localCountForGlobalCount("photonmap_mapPhotonToGrid", globalCount);

        computeEngine.executeKernel("photonmap_mapPhotonToGrid", 0, &globalCount, &localCount, 1);
        computeEngine.finish(0);
        
        double endTime = glfwGetTime();
        TSLoggerLog(std::cout, "elapsed toGrid time: ", endTime - startTime);
    
    
        //////
//        std::vector<cl_int> photonHashes(raysPerLight, -1000);
//        computeEngine.readBuffer("map_gridIndices", 0, 0, sizeof(cl_int) * raysPerLight, &photonHashes[0]);
//       
//        int validPhotons = 0;
//       
//        for (int i = 0; i < photonData.size(); i+= 9) {
//            Eigen::Vector3f position, direction, energy;
//            
//            position << photonData[i+0], photonData[i+1], photonData[i+2];
//            direction << photonData[i+3], photonData[i+4], photonData[i+5];
//            energy << photonData[i+6], photonData[i+7], photonData[i+8];
//            
//            TSLoggerLog(std::cout, "photon[", i / 9, "]={\nposition=\n", position, ", \ndirection=\n", direction, ", \nenergy=\n", energy, "}");
//            
//            int expectedHash = -1;
//            auto cellIndex = photonHashmap->getCellIndex(position);
//            if (cellIndex.x() >= 0 && cellIndex.x() < photonHashmap->xdim
//             && cellIndex.y() >= 0 && cellIndex.y() < photonHashmap->ydim
//             && cellIndex.z() >= 0 && cellIndex.z() < photonHashmap->zdim) {
//                expectedHash = photonHashmap->photonHash(cellIndex);
//            }
//            
//            TSLoggerLog(std::cout, "photonHash[", i / 9, "] = ", photonHashes[i / 9], ", should be ", expectedHash);
//            
//            TSLoggerLog(std::cout, "photonHashmap: spacing=", photonHashmap->spacing, ", xmin=", photonHashmap->xmin, ", ymin=", photonHashmap->ymin, ", zmin=", photonHashmap->zmin, ", xmax=", photonHashmap->xmax, ", ymax=", photonHashmap->ymax, ", zmin=", photonHashmap->zmin, ", xdim=", photonHashmap->xdim, ", ydim=", photonHashmap->ydim, ", zdim=", photonHashmap->zdim);
//            TSLoggerLog(std::cout, "cellIndex=", cellIndex);
//            
//            assert(photonHashes[i / 9] == expectedHash);
//            if ((i/9) > 0) {
//                assert(photonHashes[(i/9)-1] <= photonHashes[i/9]);
//            }
//            if (photonHashes[i/9] >= 0) {
//                validPhotons++;
//            }
//        }
//    
//        TSLoggerLog(std::cout, "photons in GPU map=", validPhotons);
    
    }
    
    ///
    void ocl_computeGridFirstIndices() {
    
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
            (cl_int) raysPerLight,
            
            computeEngine.getBuffer("map_gridIndices"),
            computeEngine.getBuffer("map_gridFirstPhotonIndices")
        );

        computeEngine.executeKernel("photonmap_computeGridFirstPhoton", 0, raysPerLight);
        computeEngine.finish(0);
        
        double endTime = glfwGetTime();
        TSLoggerLog(std::cout, "elapsed first index time: ", endTime - startTime);
    }
    
    ///
    void ocl_pushSceneData() {
    
        auto spheres = scene_->findElements<PovraySphere>();
        std::vector<cl_float> sphereData;
        for (auto itr = spheres.begin(); itr != spheres.end(); itr++) {
            CLPovraySphereData((*itr)->data()).writeOutData(sphereData);
        }
        
        auto planes = scene_->findElements<PovrayPlane>();
        std::vector<cl_float> planeData;
        for (auto itr = planes.begin(); itr != planes.end(); itr++) {
            CLPovrayPlaneData((*itr)->data()).writeOutData(planeData);
        }
        
        auto lights = scene_->findElements<PovrayLightSource>();
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
    
    /// Tests the usage on "ComputeEngine" following the example given at the
    /// following web address:
    ///     https://developer.apple.com/library/mac/samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html
    void ocl_raytraceRays() {
        
        unsigned int imageWidth = outputImage.width;
        unsigned int imageHeight = outputImage.height;
        void * imageData = outputImage.dataPtr();
        size_t imageDataSize = outputImage.dataSize();
        
        unsigned int rayCount = imageWidth * imageHeight;
        
        auto camera = scene_->camera();
        auto cameraData = CLPovrayCameraData(camera->data());

        computeEngine.setKernelArgs("raytrace_one_ray",
            cameraData.location,
            cameraData.up,
            cameraData.right,
            cameraData.lookAt,
           
            (cl_uint) brdfType,
            
            computeEngine.getBuffer("spheres"),
            (cl_uint) numSpheres,
            
            computeEngine.getBuffer("planes"),
            (cl_uint) numPlanes,
            
            computeEngine.getBuffer("photon_index_array"),
            computeEngine.getBuffer("photon_distance_array"),
            (cl_int) numberOfPhotonsToGather,
            
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
            (cl_int) raysPerLight, // num_photons

            computeEngine.getBuffer("map_gridIndices"),
            computeEngine.getBuffer("map_gridFirstPhotonIndices"),
           
           computeEngine.getBuffer("imageOutput"),
           (cl_uint) imageWidth,
           (cl_uint) imageHeight
        );
        
        size_t globalCount = rayCount;
        size_t localCount = localCountForGlobalCount("raytrace_one_ray", globalCount);
      
        computeEngine.executeKernel("raytrace_one_ray", 0, &globalCount, &localCount, 1);
        computeEngine.finish(0);
        computeEngine.readBuffer("imageOutput", 0, 0, imageDataSize, imageData);
        
        target.outputTexture->setNeedsUpdate();
    }
    
protected:

    friend class TealTracer;

    ///
    void setScene(std::shared_ptr<PovrayScene> scene) {
        scene_ = scene;
    }
    
private:
    ///
    std::shared_ptr<PovrayScene> scene_;

};

#endif /* GPURayTracer_hpp */
