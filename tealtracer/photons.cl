//
//  photons.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef photons_h
#define photons_h

#include "scene_config.cl"
#include "matrix_math.cl"

///
struct JensenPhoton {

    ///
    float3 position;
    float3 incomingDirection;
    
    ///
    RGBf energy;
};

struct JensenPhoton JensenPhoton_fromData(
    global const float * photon_data,
    int whichPhoton);

void JensenPhoton_setData(
    struct JensenPhoton * photon,
    // output
    global float * photon_data,
    int whichPhoton);

///
struct JensenPhoton JensenPhoton_fromData(
    global const float * photon_data,
    int whichPhoton) {
 
    struct JensenPhoton photon;
    
    global const float * photon_floats_start = &(photon_data[whichPhoton * 9]);
    
    photon.position = (float3) {
        photon_floats_start[0],
        photon_floats_start[1],
        photon_floats_start[2]
    };
    
    photon.incomingDirection = (float3) {
        photon_floats_start[3],
        photon_floats_start[4],
        photon_floats_start[5]
    };
    
    photon.energy = (float3) {
        photon_floats_start[6],
        photon_floats_start[7],
        photon_floats_start[8]
    };
    
    return photon;
}

///
void JensenPhoton_setData(
    struct JensenPhoton * photon,
    // output
    global float * photon_data,
    int whichPhoton) {

    global float * photon_floats_start = &(photon_data[whichPhoton * 9]);
    
    photon_floats_start[0] = photon->position.x;
    photon_floats_start[1] = photon->position.y;
    photon_floats_start[2] = photon->position.z;
    
    photon_floats_start[3 + 0] = photon->incomingDirection.x;
    photon_floats_start[3 + 1] = photon->incomingDirection.y;
    photon_floats_start[3 + 2] = photon->incomingDirection.z;
    
    photon_floats_start[6 + 0] = photon->energy.x;
    photon_floats_start[6 + 1] = photon->energy.y;
    photon_floats_start[6 + 2] = photon->energy.z;
}

///
void processEmittedPhoton(
    struct SceneConfig * config,
    global float * photons,
    int whichPhoton,
    
    ///
    RGBf sourceLightEnergy,
    float3 initialRayOrigin,
    float3 initialRayDirection,
    /// output
    bool * photonStored
);

///
float4 uniformSampleSphere(float u, float v);
float4 cosineSampleSphere(float u, float v);

/// Uniform sphere sampling.
float4 uniformSampleSphere(float u, float v) {
    float phi = float(2.0f * M_PI) * u;
    float cosTheta = 1.0f - 2.0f * v, sinTheta = 2.0f * sqrt(v * (1.0f - v));
    return (float4) {cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, (1.0f / (4.0f * M_PI))};
}

/// Cosine weighted sphere sampling. Up direction is the z direction.
float4 cosineSampleSphere(float u, float v) {
    const float phi = float(2.0f * M_PI) * u;
    const float vv = 2.0f * (v - 0.5f);
    const float absvv = (vv > 0? vv : -vv);
    const float cosTheta = (vv > 0? 1.0f : -1.0f) * sqrt(absvv);
    const float sinTheta = sqrt(max(0.0f,1.0f - cosTheta*cosTheta));
    
    return (float4) {cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, 2.0f * cosTheta * (1.0f/M_PI)};
}

///
void processEmittedPhoton(
    struct SceneConfig * config,
    global float * photons,
    int whichPhoton,
    
    ///
    RGBf sourceLightEnergy,
    float3 initialRayOrigin,
    float3 initialRayDirection,
    
    ///
    bool * photonStored
) {

    *photonStored = false;
    
    float3 rayOrigin = initialRayOrigin;
    float3 rayDirection = initialRayDirection;
    RGBf energy = sourceLightEnergy;
    
    struct RayIntersectionResult hit = SceneConfig_findClosestIntersection(config, initialRayOrigin, initialRayDirection);

    while (!*photonStored && hit.intersected) {
        struct JensenPhoton photon;
        
        photon.position = RayIntersectionResult_locationOfIntersection(&hit);
        photon.incomingDirection = rayDirection;
        photon.energy = energy;
        
        float value = SceneConfig_randomNormalizedFloat(config);

        if (value < config->photonBounceProbability) {

            float3 reflectedRay_direction = RayIntersectionResult_outgoingDirection(&hit);
            float3 reflectedRay_origin = RayIntersectionResult_locationOfIntersection(&hit) + 0.001f * reflectedRay_direction;
            
            //////
            struct PovrayPigment pigment;
            struct PovrayFinish finish;
    
            switch (hit.type) {
                case SphereObjectType: {
                    struct PovraySphereData data = PovraySphereData_fromData(hit.dataPtr);
                    pigment = data.pigment;
                    finish = data.finish;
                    break;
                }
                case PlaneObjectType: {
                    struct PovrayPlaneData data = PovrayPlaneData_fromData(hit.dataPtr);
                    pigment = data.pigment;
                    finish = data.finish;
                    break;
                };
                default: {
                    break;
                }
            }
    
            RGBf hitEnergy = computeOutputEnergyForBRDF(config->brdf, pigment, finish, energy, -rayDirection, reflectedRay_direction, hit.surfaceNormal) * config->photonBounceEnergyMultipler;
            //////
            
            /// Calculate intersection
            hit = SceneConfig_findClosestIntersection(config, reflectedRay_origin, reflectedRay_direction);
            rayOrigin = reflectedRay_origin;
            rayDirection = reflectedRay_direction;
            energy = hitEnergy;
        }
        else {
            JensenPhoton_setData(&photon, photons, whichPhoton);
            *photonStored = true;
        }
    }
}

#endif /* photons_h */
