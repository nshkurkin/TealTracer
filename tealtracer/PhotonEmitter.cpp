//
//  PhotonEmitter.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/16/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PhotonEmitter.hpp"

///
void
PhotonEmitter::emitPhotons(
    SingleCoreRaytracer * raytracer,
    std::vector<JensenPhoton> & photons) {
    
    /// for each light, emit photons into the scene.
    auto lights = raytracer->config.scene->findElements<PovrayLightSource>();
    float lumens = raytracer->config.lumensPerLight;
    int numRays = raytracer->config.raysPerLight;
    float luminosityPerPhoton = lumens/(float)numRays;
    
    for (int photonItr = 0; photonItr < numRays; photonItr++) {
        bool photonStored = false;
        while (!photonStored) {
            auto light = lights[raytracer->generator.randUInt() % lights.size()];
            
            float u = raytracer->generator.randFloat(), v = raytracer->generator.randFloat();
            
            Ray ray;
            ray.origin = light->position();
            ray.direction = light->getSampleDirection(u, v);
            
            processEmittedPhoton(raytracer, photons, light->color().block<3,1>(0,0) * luminosityPerPhoton, ray, &photonStored);
        }
    }
}

///
void
PhotonEmitter::processEmittedPhoton(
    SingleCoreRaytracer * raytracer,
    std::vector<JensenPhoton> & photons,
    
    ///
    RGBf sourceLightEnergy,
    const Ray & initialRay,
    
    ///
    bool * photonStored) {

    *photonStored = false;
    
    Ray ray;
    ray.origin = initialRay.origin;
    ray.direction = initialRay.direction;
    RGBf energy = sourceLightEnergy;
    
    auto hits = raytracer->config.scene->intersections(ray);

    while (!*photonStored && hits.size() > 0) {
        struct JensenPhoton photon;
        auto hit = hits[0];
        
        photon.position = hit.hit.locationOfIntersection();
        photon.incomingDirection = CompressedNormalVector3(ray.direction);
        photon.energy = rgb2rgbe(energy);
        photon.flags.geometryIndex = hit.element->id();

        float value = raytracer->generator.randFloat();
        if (value < raytracer->config.photonBounceProbability) {

            Ray reflectedRay;
            reflectedRay.direction = hit.hit.outgoingDirection();
            reflectedRay.origin = hit.hit.locationOfIntersection() + 0.001f * reflectedRay.direction;
            
            RGBf hitEnergy = raytracer->computeOutputEnergyForHit(hit, -ray.direction, hit.hit.outgoingDirection(), energy)
             * raytracer->config.photonBounceEnergyMultipler;
            //////
            
            /// Calculate intersection
            hits = raytracer->config.scene->intersections(reflectedRay);
            ray.origin = reflectedRay.origin;
            ray.direction = reflectedRay.direction;
            energy = hitEnergy;
        }
        else {
            photons.push_back(photon);
            *photonStored = true;
        }
    }
}