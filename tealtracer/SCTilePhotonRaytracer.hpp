//
//  SCTilePhotonRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SCTilePhotonRaytracer_hpp
#define SCTilePhotonRaytracer_hpp

#include "SCPhotonMapper.hpp"
#include "PhotonTiler.hpp"
#include "PhotonEmitter.hpp"

class SCTilePhotonRaytracer : public SingleCoreRaytracer {
public:

    ///
    SCTilePhotonRaytracer() : SingleCoreRaytracer() {
        photonTiler = std::shared_ptr<PhotonTiler>(new PhotonTiler());
    }
    
    /// step(x) is a geometrically distributed random number generator
    ///     representing samples of the number of trials until the first success
    ///     a Bernoulli random variable with success probability 1/x.
    int step(double x) {
        double successProbability = 1.0/x;
        int numberOfTrials = 1;
        
        while (generator.randDouble() > successProbability) {
            numberOfTrials++;
        }
        
        return numberOfTrials;
    }
    
    ///
    virtual void start() {
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
    virtual void raytraceScene() {
        
        int tileHeight = 5, tileWidth = 5;
        float photonEffectRadius = 0.25f;
        double photonSampleRate = 1.0;
        
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
                
                if (hitTest.hit.intersected) {
                
                    Eigen::Vector3f intersection = hitTest.hit.locationOfIntersection();
                    int tileIndex = photonTiler->tileIndexForPixel(outputImage.width, outputImage.height, tileWidth, tileHeight, px, py);
                    
    //                let photon[ ] = shared memory array
    //                for each pixel (x, y) in T :
    //                    load 1/n2 of tile contents into photon[ ]
                    auto & photons = photonTiler->tilePhotons[tileIndex];
    //
    //                for each pixel (x, y) in T with visible surface X:
    //                    # Iterate through tile contents
    //                    i = step(k2) − 1 # See section 2.3.2
                    int i = step(photonSampleRate) - 1;
                    int numPhotonsSampled = 0;
                    RGBf totalEnergy = RGBf::Zero();
    //                    count = 0; sum = 0
    //                    while i < photon.length:
                    while (i < photons.size()) {
    //                        ++count
                        ++numPhotonsSampled;
    //                        P = photon[i]
                        const JensenPhoton & photon = photons[i];
    //                        sum += contribution of P at X
                        if (hitTest.element->id() == photon.flags.geometryIndex) {
                            float distance = std::max<float>(0.01f, (photon.position - intersection).dot(photon.position - intersection));
                            totalEnergy += brdf->computeColor(rgbe2rgb(photon.energy), -photon.incomingDirection.vector(), -ray.direction, hitTest.hit.surfaceNormal);// / (M_PI * distance);
                        }
    //                        i += step(k2)
                        i += step(photonSampleRate);
                    }
                    
                    numPhotonsSampled = std::max<int>(1, numPhotonsSampled);
                    totalEnergy = 255.0f * totalEnergy * ((float)photons.size()) / ((float)numPhotonsSampled);
                    
                    for (int i = 0; i < 3; i++) {
                        totalEnergy(i) = std::min<float>(255.0, totalEnergy(i));
                    }
                    
                    // image[x, y] += sum ∗ photon.length / count
                    outputImage.pixel(px, py).block<3,1>(0,0) = totalEnergy.cast<uint8_t>();
                }
            }
        }
        
    }
    
    /// called in "start" to setup parameters from ".config"
    virtual void configure() {
        SingleCoreRaytracer::configure();
    }
    
protected:

    std::shared_ptr<PhotonTiler> photonTiler;

};

#endif /* SCTilePhotonRaytracer_hpp */
