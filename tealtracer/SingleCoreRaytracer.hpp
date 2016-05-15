//
//  SingleCoreRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SingleCoreRaytracer_hpp
#define SingleCoreRaytracer_hpp

#include <random>
#include <memory>

#include "BRDF.hpp"
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

    /// Puts "raytraceScene" into the ".jobPool" and then updates any view
    ///     parameters on the main thread.
    void enqueRayTrace();
    
    ///
    virtual void raytraceScene() = 0;
    
    ///
    virtual RGBf computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy);
    
    /// call this to begin the ray-tracing
    virtual void start();
    /// called in "start" to setup parameters from ".config"
    virtual void configure();
    
protected:

    std::shared_ptr<BRDF> brdf;
    
};

#endif /* SingleCoreRaytracer_hpp */
