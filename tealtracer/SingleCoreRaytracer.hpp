//
//  SingleCoreRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SingleCoreRaytracer_hpp
#define SingleCoreRaytracer_hpp

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

#include "Raytracer.hpp"

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
class SingleCoreRaytracer : public Raytracer {
public:
    
    ///
    SingleCoreRaytracer();

    ///
    void enqueRayTrace();
    
    ///
    void raytraceScene();
    
    ///
    RGBf computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy, bool usePhotonMap);
    
    ///
    void buildPhotonMap();
    
    ///
    void emitPhotons();
    ///
    void processEmittedPhoton(RGBf sourceLightEnergy, const Ray & initialRay, bool * photonStored);
    ///
    void processHits(const RGBf & energy, const Ray & ray, const std::vector<PovrayScene::InstersectionResult> & hits);
    
    /// call this to begin the ray-tracing
    virtual void start();
    
private:

    ///
    std::shared_ptr<PhotonMap> photonMap;
    std::shared_ptr<BRDF> brdf;
    
};

#endif /* SingleCoreRaytracer_hpp */
