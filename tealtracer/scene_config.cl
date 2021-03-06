//
//  scene_config.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef scene_config_h
#define scene_config_h

#include "scene_objects.cl"
#include "intersection.cl"
#include "coloring.cl"
#include "random.cl"

///
struct SceneConfig {
    
    ///
    enum BRDFType brdf;
    
    /// scene elements
    __global float * sphereData;
    unsigned int numSpheres;
    
    __global float * planeData;
    unsigned int numPlanes;
    
    // light data
    __global float * lightData;
    unsigned int numLights;
    float luminosityPerPhoton;
    float photonBounceProbability;
    float photonBounceEnergyMultipler;
    
    /// miscellaneous
    struct RandomGenerator generator;
};

///
struct RayIntersectionResult SceneConfig_findClosestIntersection(struct SceneConfig * config, float3 rayOrigin, float3 rayDirection);

///
struct RayIntersectionResult SceneConfig_findClosestIntersection(struct SceneConfig * config, float3 rayOrigin, float3 rayDirection) {
    /// Calculate intersections
    struct RayIntersectionResult bestIntersection;
    bestIntersection.intersected = false;
    bestIntersection.timeOfIntersection = INFINITY;
    
    __global float * dataPtrs[2] = { config->sphereData, config->planeData };
    unsigned int dataCounts[2] = { config->numSpheres, config->numPlanes };
    unsigned int dataStrides[2] = { kPovraySphereStride, kPovrayPlaneStride };
    
    for (unsigned int i = 0; i < (unsigned int) NumObjectTypes; i++) {
        struct RayIntersectionResult intersection = closest_intersection(
            dataPtrs[i], dataCounts[i], dataStrides[i],
            rayOrigin, rayDirection,
            (enum ObjectType) i);
        
        if (intersection.intersected
         && intersection.timeOfIntersection < bestIntersection.timeOfIntersection) {
            bestIntersection = intersection;
        }
    }
    
    return bestIntersection;
}

#endif /* scene_config_h */
