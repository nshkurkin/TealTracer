//
//  SCPhotonMapper.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SCPhotonMapper.hpp"

#include "PhotonHashmap.hpp"
#include "PhotonKDTree.hpp"
#include "PhotonEmitter.hpp"

///
void
SCPhotonMapper::configure() {

    SingleCoreRaytracer::configure();

    switch (config.supportedPhotonMap) {
    case RaytracingConfig::KDTree: {
        auto * map = new PhotonKDTree();
        photonMap = std::shared_ptr<PhotonMap>(map);
        break;
    }
    case RaytracingConfig::HashGrid: {
        auto * map = new PhotonHashmap();
        map->cellsize = config.hashmapCellsize;
        map->spacing = config.hashmapSpacing;
        map->setDimensions(config.hashmapGridStart, config.hashmapGridEnd);
        photonMap = std::shared_ptr<PhotonMap>(map);
        break;
    }
    default:
        assert(false);
        break;
    }
}

///
void
SCPhotonMapper::start() {

    configure();
    
    jobPool.emplaceJob(JobPool::WorkItem("[CPU] Build photon map", [=]() {
        TSLoggerLog(std::cout, "[", glfwGetTime(), "] Started building photon map");
        assert(photonMap != nullptr);
        PhotonEmitter().emitPhotons(this, photonMap->photons);
        photonMap->buildMap();
        TSLoggerLog(std::cout, "Done emplacing");
    }, [=]() {
        TSLoggerLog(std::cout, "[", glfwGetTime(), "] Finished building photon map");
        this->enqueRayTrace();
    }));
}


///
void
SCPhotonMapper::raytraceScene() {
    assert(config.scene != nullptr);
    
    /// Get the camera
    auto camera = config.scene->camera();
    auto lights = config.scene->findElements<PovrayLightSource>();
    
    /// Create all of the rays
    auto camPos = camera->location();    
    auto frame = camera->basisVectors();
    
    for (int px = 0; px < outputImage.width; px++) {
        for (int py = 0; py < outputImage.height; py++) {
            Ray ray;
            ray.origin = camPos;
            ray.direction = (frame.forward - 0.5*frame.up - 0.5*frame.right + frame.right*(0.5+(double)px)/(double)outputImage.width + frame.up*(0.5+(double)py)/(double)outputImage.height).normalized();
            
            auto hitTest = config.scene->closestIntersection(ray);
            Image<uint8_t>::Vector4 color = Image<uint8_t>::Vector4(0, 0, 0, 255);
            
            if (hitTest.element != nullptr && hitTest.element->pigment() != nullptr) {
                /// Get indirect lighting
                RGBf result = RGBf(0,0,0);
                result += 255.0 * computeOutputEnergyForHitUsingPhotonMap(hitTest, -ray.direction, RGBf(1,1,1));
                
                for (int i = 0; i < 3; i++) {
                    result(i) = std::min<float>(255.0, result(i));
                }
                
                color.block<3,1>(0,0) = result.cast<uint8_t>();
            }
            
            outputImage.pixel(px, py) = color;
        }
    }
}

///
RGBf
SCPhotonMapper::computeOutputEnergyForHitUsingPhotonMap(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy) {
    
    RGBf output = RGBf::Zero();
    brdf->pigment = *hitResult.element->pigment();
    brdf->finish = *hitResult.element->finish();
    
    auto photonInfo = photonMap->gatherPhotonsIndices(config.numberOfPhotonsToGather, config.maxPhotonGatherDistance, hitResult.hit.locationOfIntersection());
    
    float maxSqrDist = 0.001;
    //  Accumulate radiance of the K nearest photons
    for (int i = 0; i < photonInfo.size(); ++i) {
        
        const auto & p = photonMap->photons[photonInfo[i].index];
        
        RGBf photonEnergy = RGBf::Zero();
        if (photonInfo[i].squareDistance > maxSqrDist) {
            maxSqrDist = photonInfo[i].squareDistance;
        }
        
        photonEnergy = brdf->computeColor(rgbe2rgb(p.energy), -p.incomingDirection.vector(), toViewer, hitResult.hit.surfaceNormal);
        
        output += photonEnergy;
    }
    
    output = output / (M_PI * maxSqrDist);
    return output;
}
