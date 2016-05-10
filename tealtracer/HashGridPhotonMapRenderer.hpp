//
//  HashGridPhotonMapRenderer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef HashGridPhotonMapRenderer_hpp
#define HashGridPhotonMapRenderer_hpp

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
#include "RaytracingConfig.hpp"


class HashGridPhotonMapRenderer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:

    ///
    /// Rendering pipeline setup:
    ///     1) Have an OpenGL texture to display on screen, connect it to GPU
    ///     2) Perform raycasts on the GPU and fill the texture with content
    ///     3) Display the texture
    ///
    
    ///
    HashGridPhotonMapRenderer();

    ///
    virtual void setupDrawingInWindow(TSWindow * window);

    void start();
    
    ///
    void ocl_raytraceSetup();
    ///
    void ocl_pushSceneData();
    
    ///
    void ocl_buildPhotonMap();
    ///
    void ocl_emitPhotons();
    ///
    void ocl_sortPhotons();
    ///
    void ocl_mapPhotonsToGrid();
    ///
    void ocl_computeGridFirstIndices();
    
    ///
    void enqueRayTrace();
    /// Tests the usage on "ComputeEngine" following the example given at the
    /// following web address:
    ///     https://developer.apple.com/library/mac/samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html
    void ocl_raytraceRays();

    ///
    virtual void drawInWindow(TSWindow * window);
    ///
    virtual void windowResize(TSWindow * window, int w, int h);
    ///
    virtual void framebufferResize(TSWindow * window, int w, int h);
    ///
    virtual void windowClose(TSWindow * window);
    ///
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods);
    ///
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods);
    ///
    virtual void mouseUp(TSWindow * window, int button, int mods);
    ///
    virtual void mouseDown(TSWindow * window, int button, int mods);
    ///
    virtual void mouseMoved(TSWindow * window, double x, double y);
    ///
    virtual void mouseScroll(TSWindow * window, double dx, double dy);

private:

    ComputeEngine computeEngine;
    TextureRenderTarget target;
    
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<float> distribution;
    
    Image<uint8_t> outputImage;

    JobPool jobPool;
    int framesRendered;
    double lastRayTraceTime, rayTraceElapsedTime;
    
    RaytracingConfig config;
    std::shared_ptr<PhotonHashmap> photonHashmap;
    
    bool useGPU;
    unsigned int numSpheres, numPlanes, numLights;
    
    float FPSsaved, realtimeSaved;
    double lastX, lastY;

};

#endif /* HashGridPhotonMapRenderer_hpp */
