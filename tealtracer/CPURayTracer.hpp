//
//  CPURayTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef CPURayTracer_hpp
#define CPURayTracer_hpp

#include "TSWindow.hpp"
#include "gl_include.h"
#include "opengl_errors.hpp"
#include "compute_engine.hpp"
#include "stl_extensions.hpp"

#include "OpenGLShaders.hpp"
#include "PovrayScene.hpp"
#include "TSLogger.hpp"
#include "Image.hpp"
#include "JobPool.hpp"
#include "TextureRenderTarget.hpp"

#include "BRDF.hpp"

#include "PhotonMap.hpp"
#include "PhotonHashmap.hpp"

#include <random>
#include <memory>

/// From Lab 1:
///
///     *) Parse the scne description file
///     *) Computing ray-object intersections
///     *) Shading
///     *) Recursive Tracing (reflection, refraction, shadows)
///     *) Write out resulting image
///

Eigen::Matrix4f lookAt(const Eigen::Vector3f & eye, const Eigen::Vector3f & center, const Eigen::Vector3f & up);

///
class CPURayTracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:
    
    ///
    /// Rendering pipeline:
    ///
    ///     1) Queue a Raytrace into "outputImage"
    ///     2) When complete, copy data from "outputImage" to "outputTexture"
    ///     3) Render "outputTexture"
    ///

    Image<uint8_t> outputImage;
    TextureRenderTarget target;
    JobPool jobPool;
    
    int renderOutputWidth;
    int renderOutputHeight;

    enum SupportedPhotonMap {
        KDTree = 0,
        Hashmap = 1
    };
    
    enum SupportedBRDF {
        BlinnPhong = 0, // https://en.wikipedia.org/wiki/Blinn–Phong_shading_model
        OrenNayar = 1 // https://en.wikipedia.org/wiki/Oren–Nayar_reflectance_model
    };
    
    SupportedBRDF brdfType;
    std::shared_ptr<BRDF> brdf;

    int numberOfPhotonsToGather;
    float maxPhotonGatherDistance;
    int raysPerLight;
    int lumensPerLight;
    SupportedPhotonMap photonMapType;

    float photonBounceProbability;
    float photonBounceEnergyMultipler;

    bool mapShadowPhotons;
    
    Eigen::Vector3f hashmapGridStart, hashmapGridEnd;
    float hashmapSpacing, hashmapCellsize;
    
    CPURayTracer();

    ///
    virtual void setupDrawingInWindow(TSWindow * window);

    int framesRendered;
    double lastRayTraceTime, rayTraceElapsedTime;

    ///
    void enqueRayTrace();

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
    
    static const Eigen::Vector3f Up;
    static const Eigen::Vector3f Forward;
    static const Eigen::Vector3f Right;
    
    ///
    void raytraceScene();
    
    ///
    RGBf computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy, bool usePhotonMap);
    
    ///
    std::shared_ptr<PhotonMap> photonMap;
    
    ///
    void enqueuePhotonMapping();
    ///
    void buildPhotonMap();
    
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution;
    
    ///
    void emitPhotons();
    
    ///
    void processEmittedPhoton(RGBf sourceLightEnergy, const Ray & initialRay, bool * photonStored);
    ///
    void processHits(const RGBf & energy, const Ray & ray, const std::vector<PovrayScene::InstersectionResult> & hits);
    
    /// call this to begin the ray-tracing
    void start();
    
protected:

    friend class TealTracer;

    ///
    void setScene(std::shared_ptr<PovrayScene> scene);
    
private:
    ///
    std::shared_ptr<PovrayScene> scene_;

    
};

#endif /* CPURayTracer_hpp */
