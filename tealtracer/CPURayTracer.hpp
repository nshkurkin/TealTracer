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
#include "PhotonMap.hpp"

#include <random>

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

    Image outputImage;
    TextureRenderTarget target;
    JobPool jobPool;
    
    int renderOutputWidth;
    int renderOutputHeight;

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
    RGBf computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult);
    
    ///
    PhotonMap photonMap;
    
    ///
    void enqueuePhotonMapping() {
        jobPool.emplaceJob([=]() {
            TSLoggerLog(std::cout, "[", glfwGetTime(), "] Started building photon map");
            buildPhotonMap();
        }, [=]() {
            TSLoggerLog(std::cout, "[", glfwGetTime(), "] Finished building photon map");
        });
    }
    
    ///
    void buildPhotonMap() {
        photonMap.setDimensions(Eigen::Vector3f(-20,-20,-20), Eigen::Vector3f(20,20,20));
        photonMap.photons.clear();
        emitPhotons();
        photonMap.buildSpatialHash();
    }
    
    ///
    void emitPhotons() {
        /// for each light, emit photons into the scene.
        auto lights = scene_->findElements<PovrayLightSource>();
        for (auto itr = lights.begin(); itr != lights.end(); itr++) {
            auto light = *itr;
            auto color = light->color();
            
            std::default_random_engine generator;
            std::uniform_real_distribution<float> distribution(0.0,1.0);
  
            for (int i = 0; i < 100000; i++) {
                float u = distribution(generator);
                float v = distribution(generator);
                
                Ray ray;
                
                ray.origin = light->position();
                ray.direction = light->getSampleDirection(u, v);
                
                auto hits = scene_->intersections(ray);
                /// Add in shadow photons
                for (int i = 1; i < hits.size(); i++) {
                    const auto & hitResult = hits[i];
                    photonMap.photons.push_back(JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, RGBf::Zero(), true, false, hitResult.element->id()));
                }
                /// bounce around the other photon
                if (hits.size() > 0) {
                    const auto & hitResult = hits[0];
                    JensenPhoton photon = JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, color.block<3,1>(0,0), false, false, hitResult.element->id());
                    bouncePhoton(photon);
                    photonMap.photons.push_back(photon);
                }
            }
        }
        
        TSLoggerLog(std::cout, "Photons=", photonMap.photons.size());
    }
    
    ///
    void bouncePhoton(JensenPhoton & photon) {
        
    }
    
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
