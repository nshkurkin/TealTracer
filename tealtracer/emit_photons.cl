//
//  emit_photons.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef emit_photons_h
#define emit_photons_h

#include "intersection.cl"
#include "random.cl"

/// Uniform sphere sampling.
float4 uniformSampleSphere(const float & u, const float & v) {
    float phi = float(2.0 * M_PI) * u;
    float cosTheta = 1.0f - 2.0f * v, sinTheta = 2.0f * sqrt(v * (1.0f - v));
    return (float4) {cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, (1.0f / (4.0f * M_PI))};
}

/// Cosine weighted sphere sampling. Up direction is the z direction.
float4 cosineSampleSphere(float u, float v) {
    const float phi = float(2.0*M_PI) * u;
    const float vv = 2.0f*(v-0.5f);
    const float cosTheta = (vv > 0? 1.0 : -1.0)*sqrt(std::abs(vv));
    const float sinTheta = sqrt(max(0.0f,1.0f - cosTheta*cosTheta));
    
    return (float4) {cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, 2.0f*cosTheta*(1.0f/M_PI)};
}

///
global kernel void emit_photon(
    // light data
    __global float * lightData,
    const unsigned int numLights,
    
    const float luminosityPerPhoton // lumens/(float)numRays
    //
    ) {
    
    int whichPhoton = (int) get_global_id(0);
    
    struct mwc64x_state_t generator;
    MWC64X_SeedStreams(&generator, (ulong) get_global_id(0), (ulong) get_global_size(0));
    
    /// choose a random light and direction
    int whichLight = ((int) MWC64X_NextUint(&generator)) % numLights;
    struct PovrayLightSourceData light = PovrayLightSourceData_fromData(whichLight * lightData[kPovrayLightSourceStride]);
    
    float u = ((float) (((int) MWC64X_NextUint(&generator)) % 100000)) / 100000.0f;
    float v = ((float) (((int) MWC64X_NextUint(&generator)) % 100000)) / 100000.0f;
    
    float3 rayOrigin = light.position;
    float3 rayDirection = cosineSampleSphere(u, v).xyz;
    
    /// for each light, emit photons into the scene.
//    auto lights = scene_->findElements<PovrayLightSource>();
//    for (auto itr = lights.begin(); itr != lights.end(); itr++) {
//        auto light = *itr;
//        auto color = light->color();
//
//        float lumens = lumensPerLight;
//        int numRays = raysPerLight;
//        float luminosityPerPhoton = lumens/(float)numRays;
//
//        for (int i = 0; i < numRays; i++) {
//            float u = distribution(generator);
//            float v = distribution(generator);
//            
//            Ray ray;
//            
//            ray.origin = light->position();
//            ray.direction = light->getSampleDirection(u, v);
//            
//            auto hits = scene_->intersections(ray);
//            processHits(color.block<3,1>(0,0) * luminosityPerPhoton, ray, hits);
//        }
//    }
//    
//    TSLoggerLog(std::cout, "Photons=", photonMap->photons.size());
}

///
//void processHits(const RGBf & energy, const Ray & ray, const std::vector<PovrayScene::InstersectionResult> & hits) {
//    /// Add in shadow photons
//    for (int i = 1; i < hits.size(); i++) {
//        const auto & hitResult = hits[i];
//        photonMap->photons.push_back(JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, RGBf::Zero(), true, false, hitResult.element->id()));
//    }
//    /// bounce around the other photon
//    if (hits.size() > 0) {
//        const auto & hitResult = hits[0];
//        JensenPhoton photon = JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, energy, false, false, hitResult.element->id());
//        
//        if (distribution(generator) < photonBounceProbability) {
//            Ray reflectedRay;
//            
//            reflectedRay.direction = hitResult.hit.outgoingDirection();
//            reflectedRay.origin = hitResult.hit.locationOfIntersection() + 0.001 * reflectedRay.direction;
//            
//            auto hitEnergy = computeOutputEnergyForHit(hitResult, -ray.direction, hitResult.hit.outgoingDirection(), false) * photonBounceEnergyMultipler;
//            auto newHits = scene_->intersections(reflectedRay);
//            
//            processHits(hitEnergy, reflectedRay, newHits);
//        }
//        else {
//            photonMap->photons.push_back(photon);
//        }
//    }
//}

#endif /* emit_photons_h */
