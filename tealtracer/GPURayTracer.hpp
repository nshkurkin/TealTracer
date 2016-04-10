//
//  GPURayTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef GPURayTracer_hpp
#define GPURayTracer_hpp

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
    
    Image outputImage;

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
        
        outputImage.setDimensions(window->width(), window->height());
        target.init(outputImage.width, outputImage.height, outputImage.dataPtr());
        
        FPSsaved = 0.0;
        
        /// OpenCL
        ocl_raytraceSetup();
        ocl_pushSceneData();
        enqueRayTrace();
        
        ///
        lastX = std::numeric_limits<float>::infinity();
        lastY = std::numeric_limits<float>::infinity();
    }

    float FPSsaved;

    void start() {
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
            FPSsaved = FPS;
        }
        
        window->setTitle(make_string("GPU Ray Tracer (FPS: ", FPSsaved, ", frames: ", framesRendered, ")"));
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
    unsigned int numSpheres, numPlanes;
    
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
        
        auto camera = scene_->camera();
        
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
        
        if (spheres.size() > 0) {
            computeEngine.createBuffer("spheres", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * sphereData.size());
        }
        if (planes.size() > 0) {
            computeEngine.createBuffer("planes", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(cl_float) * planeData.size());
        }
        
        computeEngine.createBuffer("imageOutput", ComputeEngine::MemFlags::MEM_WRITE_ONLY, imageDataSize);
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
    
        numSpheres = (unsigned int) spheres.size();
        numPlanes = (unsigned int) planes.size();
        
        if (spheres.size() > 0) {
            computeEngine.writeBuffer("spheres", 0, 0, sizeof(cl_float) * sphereData.size(), &sphereData[0]);
        }
        if (planes.size() > 0) {
            computeEngine.writeBuffer("planes", 0, 0, sizeof(cl_float) * planeData.size(), &planeData[0]);
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
           
           computeEngine.getBuffer("spheres"),
           (cl_uint) numSpheres,

           computeEngine.getBuffer("planes"),
           (cl_uint) numPlanes,
           
           computeEngine.getBuffer("imageOutput"),
           (cl_uint) imageWidth,
           (cl_uint) imageHeight
        );
        
        size_t globalCount = rayCount;
        size_t localCount = imageWidth;
        if (computeEngine.requestedDeviceType == ComputeEngine::DeviceType::DEVICE_TYPE_CPU) {
            localCount = 20;
        }
      
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
