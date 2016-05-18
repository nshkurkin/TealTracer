//
//  SCTilePhotonRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SCTilePhotonRaytracer.hpp"
#include "PhotonEmitter.hpp"

///
SCTilePhotonRaytracer::SCTilePhotonRaytracer() : SingleCoreRaytracer() {
    photonTiler = std::shared_ptr<PhotonTiler>(new PhotonTiler());
}

///
int
SCTilePhotonRaytracer::step(double x) {
    double successProbability = 1.0/x;
    int numberOfTrials = 1;
    
    while (generator.randDouble() > successProbability) {
        numberOfTrials++;
    }
    
    return numberOfTrials;
}

///
void
SCTilePhotonRaytracer::start() {
    configure();

    jobPool.emplaceJob(JobPool::WorkItem("[CPU] Build photon map", [=]() {
        double t0 = glfwGetTime();
        TSLoggerLog(std::cout, "Started emitting photons");
        PhotonEmitter().emitPhotons(this, photonTiler->photons);
        double tf = glfwGetTime();
        TSLoggerLog(std::cout, "Done emitting photons (t=", tf - t0, ")");
    }, [=]() {
        this->enqueRayTrace();
    }));
}

///
void
SCTilePhotonRaytracer::raytraceScene() {
    
    int tileHeight = config.tile_height, tileWidth = config.tile_width;
    float photonEffectRadius = config.tile_photonEffectRadius;
    double photonSampleRate = config.tile_photonSampleRate;
    
    Eigen::Vector3f cameraPosition = config.scene->camera()->location();
    FrenetFrame frame = config.scene->camera()->basisVectors();
    
    photonTiler->generateTiles(outputImage.width, outputImage.height, tileWidth, tileHeight, config.scene->camera()->location(), config.scene->camera()->basisVectors());
    photonTiler->buildMap(photonEffectRadius);
    
    for (int py = 0; py < outputImage.height; py++) {
        for (int px = 0; px < outputImage.width; px++) {
            Ray ray;
            ray.origin = cameraPosition;
            ray.direction = (frame.forward - 0.5*frame.up - 0.5*frame.right + frame.right*(0.5+(double)px)/(double)outputImage.width + frame.up*(0.5+(double)py)/(double)outputImage.height).normalized();
            
            auto hitTest = config.scene->closestIntersection(ray);
            
            RGBf totalEnergy = RGBf::Zero();
            
            if (hitTest.hit.intersected) {
            
                Eigen::Vector3f intersection = hitTest.hit.locationOfIntersection();
                int tileIndex = photonTiler->tileIndexForPixel(outputImage.width, outputImage.height, tileWidth, tileHeight, px, py);
                
                brdf->pigment = *hitTest.element->pigment();
                brdf->finish = *hitTest.element->finish();
                
                auto & photons = photonTiler->tilePhotons[tileIndex];
                
                int i = step(photonSampleRate) - 1;
                int numPhotonsSampled = 0;
                float maxDistanceSqd = -std::numeric_limits<float>::infinity();
                
                /// Sample the collection of photons
                while (i < photons.size()) {
                    const JensenPhoton & photon = photons[i];
                    float distanceSqrd = (photon.position - intersection).dot(photon.position - intersection);
                    if (hitTest.element->id() == photon.flags.geometryIndex
                     && distanceSqrd <= photonEffectRadius * photonEffectRadius) {
                        ++numPhotonsSampled;
                        totalEnergy += brdf->computeColor(rgbe2rgb(photon.energy), -photon.incomingDirection.vector(), -ray.direction, hitTest.hit.surfaceNormal);
                        maxDistanceSqd = std::max<float>(maxDistanceSqd, distanceSqrd);
                    }
                    i += step(photonSampleRate);
                }
                
                if (numPhotonsSampled > 0) {
                    totalEnergy = 255.0f * totalEnergy * (1.0f/(M_PI * maxDistanceSqd));
                    
                    for (int i = 0; i < 3; i++) {
                        totalEnergy(i) = std::min<float>(255.0, totalEnergy(i));
                    }
                }
            }
            
            outputImage.pixel(px, py).block<3,1>(0,0) = totalEnergy.cast<uint8_t>();
        }
    }
    
}
