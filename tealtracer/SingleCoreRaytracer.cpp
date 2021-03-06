//
//  SingleCoreRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SingleCoreRaytracer.hpp"

#include "opengl_errors.hpp"
#include "stl_extensions.hpp"

#include "TSLogger.hpp"

///
SingleCoreRaytracer::SingleCoreRaytracer() {
    brdf = nullptr;
}

///
void
SingleCoreRaytracer::configure() {
    
    switch (config.brdfType) {
    case RaytracingConfig::BlinnPhong: {
        brdf = std::shared_ptr<BRDF>(new BlinnPhongBRDF());
        break;
    }
    case RaytracingConfig::OrenNayar: {
        brdf = std::shared_ptr<BRDF>(new OrenNayarBRDF());
        break;
    }
    default:
        assert(false);
        break;
    }
    
    lastRayTraceTime = glfwGetTime();
    rayTraceElapsedTime = 0.0;
    framesRendered = 0;
}

///
void
SingleCoreRaytracer::start() {

    configure();
    this->enqueRayTrace();
}

///
void
SingleCoreRaytracer::enqueRayTrace() {
    jobPool.emplaceJob(JobPool::WorkItem("[CPU] Raytrace", [=](){
//        TSLoggerLog(std::cout, "Beginning ray trace");
        auto startTime = glfwGetTime();
        this->raytraceScene();
        auto endTime = glfwGetTime();
        lastRayTraceTime = endTime - startTime;
    }, [=](){
//        TSLoggerLog(std::cout, "Finished ray trace");
        rayTraceElapsedTime = lastRayTraceTime;
        framesRendered++;
        this->target.outputTexture->setNeedsUpdate();
        this->enqueRayTrace();
    }));
}

///
RGBf
SingleCoreRaytracer::computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy) {
    
    brdf->pigment = *hitResult.element->pigment();
    brdf->finish = *hitResult.element->finish();
    
    return brdf->computeColor(sourceEnergy, toLight, toViewer, hitResult.hit.surfaceNormal);
}
