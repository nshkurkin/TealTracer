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

///
void
SCPhotonMapper::buildPhotonMap() {
    assert(photonMap != nullptr);
    photonMap->photons.clear();
    emitPhotons();
    photonMap->buildMap();
}

///
void
SCPhotonMapper::emitPhotons() {
    /// for each light, emit photons into the scene.
    auto lights = config.scene->findElements<PovrayLightSource>();
    float lumens = config.lumensPerLight;
    int numRays = config.raysPerLight;
    float luminosityPerPhoton = lumens/(float)numRays;
    
    for (int photonItr = 0; photonItr < numRays; photonItr++) {
        bool photonStored = false;
        while (!photonStored) {
            auto light = lights[generator.randUInt() % lights.size()];
            
            float u = generator.randFloat(), v = generator.randFloat();
            
            Ray ray;
            ray.origin = light->position();
            ray.direction = light->getSampleDirection(u, v);
            
            processEmittedPhoton(light->color().block<3,1>(0,0) * luminosityPerPhoton, ray, &photonStored);
        }
    }
    
    TSLoggerLog(std::cout, "Photons=", photonMap->photons.size());
}

///
void
SCPhotonMapper::processEmittedPhoton(
    ///
    RGBf sourceLightEnergy,
    const Ray & initialRay,
    
    ///
    bool * photonStored
    ) {

    *photonStored = false;
    
    Ray ray;
    ray.origin = initialRay.origin;
    ray.direction = initialRay.direction;
    RGBf energy = sourceLightEnergy;
    
    auto hits = config.scene->intersections(ray);
    bool firstHit = !config.usePhotonMappingForDirectIllumination;

    while (!*photonStored && hits.size() > 0) {
        struct JensenPhoton photon;
        auto hit = hits[0];
        
        photon.position = hit.hit.locationOfIntersection();
        photon.incomingDirection = CompressedNormalVector3(ray.direction);
        photon.energy = rgb2rgbe(energy);
        photon.flags.geometryIndex = hit.element->id();

        float value = generator.randFloat();
        if (value < config.photonBounceProbability || firstHit) {

            Ray reflectedRay;
            reflectedRay.direction = hit.hit.outgoingDirection();
            reflectedRay.origin = hit.hit.locationOfIntersection() + 0.001f * reflectedRay.direction;
            
            RGBf hitEnergy = computeOutputEnergyForHit(hit, -ray.direction, hit.hit.outgoingDirection(), energy)
             * config.photonBounceEnergyMultipler;
            //////
            
            /// Calculate intersection
            hits = config.scene->intersections(reflectedRay);
            ray.origin = reflectedRay.origin;
            ray.direction = reflectedRay.direction;
            energy = hitEnergy;
            
            firstHit = false;
        }
        else {
            photonMap->photons.push_back(photon);
            *photonStored = true;
        }
    }
}

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
        buildPhotonMap();
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
    auto viewTransform = lookAt(camera->location(), camera->lookAt(), camera->up());
    Eigen::Vector3f forward = (viewTransform * Eigen::Vector4f(config.Forward.x(), config.Forward.y(), config.Forward.z(), 0.0)).block<3,1>(0,0);
    Eigen::Vector3f up = (viewTransform * Eigen::Vector4f(config.Up.x(), config.Up.y(), config.Up.z(), 0.0)).block<3,1>(0,0) * camera->up().norm();
    Eigen::Vector3f right = (viewTransform * Eigen::Vector4f(config.Right.x(), config.Right.y(), config.Right.z(), 0.0)).block<3,1>(0,0) * camera->right().norm();
    
    for (int px = 0; px < outputImage.width; px++) {
        for (int py = 0; py < outputImage.height; py++) {
            Ray ray;
            ray.origin = camPos;
            ray.direction = (forward - 0.5*up - 0.5*right + right*(0.5+(double)px)/(double)outputImage.width + up*(0.5+(double)py)/(double)outputImage.height).normalized();
            
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
