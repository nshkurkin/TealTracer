//
//  SingleCoreRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SingleCoreRaytracer.hpp"

#include "PhotonKDTree.hpp"

///
Eigen::Matrix4f
lookAt(const Eigen::Vector3f & eye, const Eigen::Vector3f & center, const Eigen::Vector3f & up) {
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);
    Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
    
    result(0,0) = s.x();
    result(1,0) = s.y();
    result(2,0) = s.z();
    result(0,1) = u.x();
    result(1,1) = u.y();
    result(2,1) = u.z();
    result(0,2) = -f.x();
    result(1,2) = -f.y();
    result(2,2) = -f.z();
    
    result(3,0) = -s.dot(eye);
    result(3,1) = -u.dot(eye);
    result(3,2) =  f.dot(eye);
    
    return result;
}

///
SingleCoreRaytracer::SingleCoreRaytracer() {
    photonMap = nullptr;
    brdf = nullptr;
}

///
void
SingleCoreRaytracer::start() {

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
void
SingleCoreRaytracer::buildPhotonMap() {
    assert(photonMap != nullptr);
    photonMap->photons.clear();
    emitPhotons();
    photonMap->buildMap();
}

///
void
SingleCoreRaytracer::emitPhotons() {
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
SingleCoreRaytracer::processEmittedPhoton(
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
            
            RGBf hitEnergy = computeOutputEnergyForHit(hit, -ray.direction, hit.hit.outgoingDirection(), energy, false)
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
SingleCoreRaytracer::processHits(const RGBf & energy, const Ray & ray, const std::vector<PovrayScene::InstersectionResult> & hits) {
    /// Add in shadow photons
    for (int i = 1; i < hits.size(); i++) {
        const auto & hitResult = hits[i];
        photonMap->photons.push_back(JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, RGBf::Zero(), true, false, hitResult.element->id()));
    }

    /// bounce around the other photon
    if (hits.size() > 0) {
        const auto & hitResult = hits[0];
        JensenPhoton photon = JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, energy, false, false, hitResult.element->id());
        
        if (generator.randFloat() < config.photonBounceProbability) {
            Ray reflectedRay;
            
            reflectedRay.direction = hitResult.hit.outgoingDirection();
            reflectedRay.origin = hitResult.hit.locationOfIntersection() + 0.001f * reflectedRay.direction;
            
            auto hitEnergy = computeOutputEnergyForHit(hitResult, -ray.direction, hitResult.hit.outgoingDirection(), energy, false) * config.photonBounceEnergyMultipler;
            auto newHits = config.scene->intersections(reflectedRay);
            
            processHits(hitEnergy, reflectedRay, newHits);
        }
        else {
            photonMap->photons.push_back(photon);
        }
    }
}

///
void SingleCoreRaytracer::raytraceScene() {
    assert(config.scene != nullptr);
    
    /// Get the camera
    auto camera = config.scene->camera();
    auto lights = config.scene->findElements<PovrayLightSource>();
    
    /// TODO: build the photon map
    
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
                
                if (config.indirectIlluminationEnabled || config.usePhotonMappingForDirectIllumination) {
                    result += 255.0 * computeOutputEnergyForHit(hitTest, Eigen::Vector3f::Zero(), -ray.direction, RGBf(1,1,1), true);
                }
                
                /// Get direct lighting
                if (config.directIlluminationEnabled && !config.usePhotonMappingForDirectIllumination) {
                    for (auto lightItr = lights.begin(); lightItr != lights.end(); lightItr++) {
                        auto light = *lightItr;
                        Eigen::Vector3f hitLoc = hitTest.hit.locationOfIntersection();
                        Eigen::Vector3f toLight = light->position() - hitLoc;
                        Eigen::Vector3f toLightDir = toLight.normalized();
                        Ray shadowRay;
                        shadowRay.origin = hitLoc + 0.01f * toLightDir;
                        shadowRay.direction = toLightDir;
                        auto shadowHitTest = config.scene->closestIntersection(shadowRay);
                        
                        bool isShadowed = config.shadowsEnabled && !(!shadowHitTest.hit.intersected
                         || (shadowHitTest.hit.intersected && shadowHitTest.hit.timeOfIntersection > toLight.norm()));
                        
                        if (!isShadowed) {
                            result += 255.0 * computeOutputEnergyForHit(hitTest, toLightDir, (camPos - hitLoc).normalized(), light->color().block<3,1>(0,0), false);
                        }
                    }
                }
                
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
SingleCoreRaytracer::computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy, bool usePhotonMap) {
    
    RGBf output = RGBf::Zero();
    brdf->pigment = *hitResult.element->pigment();
    brdf->finish = *hitResult.element->finish();
    
    if (usePhotonMap) {
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
        
    }
    else {
       output = brdf->computeColor(sourceEnergy, toLight, toViewer, hitResult.hit.surfaceNormal);
    }
    
    return output;
}
