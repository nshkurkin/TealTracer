//
//  emit_photons.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef emit_photons_h
#define emit_photons_h

//#include "intersection.cl"
//#include "random.cl"
//
///// Uniform sphere sampling.
//float4 uniformSampleSphere(const float & u, const float & v) {
//    float phi = float(2.0 * M_PI) * u;
//    float cosTheta = 1.0f - 2.0f * v, sinTheta = 2.0f * sqrt(v * (1.0f - v));
//    return (float4) {cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, (1.0f / (4.0f * M_PI))};
//}
//
///// Cosine weighted sphere sampling. Up direction is the z direction.
//float4 cosineSampleSphere(float u, float v) {
//    const float phi = float(2.0*M_PI) * u;
//    const float vv = 2.0f*(v-0.5f);
//    const float cosTheta = (vv > 0? 1.0 : -1.0)*sqrt(std::abs(vv));
//    const float sinTheta = sqrt(max(0.0f,1.0f - cosTheta*cosTheta));
//    
//    return (float4) {cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, 2.0f*cosTheta*(1.0f/M_PI)};
//}
//
/////
//struct SceneConfig {
//    ///
//    enum BRDFType brdf;
//    
//    /// scene elements
//    __global float * sphereData;
//    unsigned int numSpheres;
//    
//    __global float * planeData;
//    unsigned int numPlanes;
//    
//    // light data
//    __global float * lightData;
//    unsigned int numLights;
//    float luminosityPerPhoton;
//    float photonBounceProbability;
//    float photonBounceEnergyMultipler;
//    
//    // photon map data
//    struct PhotonHashmap map;
//    
//    /// miscellaneous
//    struct mwc64x_state_t generator;
//};
//
/////
//struct RayIntersectionResult SceneConfig_findClosestIntersection(struct SceneConfig * config, float3 rayOrigin, float3 rayDirection) {
//    /// Calculate intersections
//    struct RayIntersectionResult bestIntersection;
//    bestIntersection.intersected = false;
//    bestIntersection.timeOfIntersection = INFINITY;
//    
//    __global float * dataPtrs[2] = { config->sphereData, config->planeData };
//    unsigned int dataCounts[2] = { config->numSpheres, config->numPlanes };
//    unsigned int dataStrides[2] = { kPovraySphereStride, kPovrayPlaneStride };
//    
//    for (unsigned int i = 0; i < (unsigned int) NumObjectTypes; i++) {
//        struct RayIntersectionResult intersection = closest_intersection(
//            dataPtrs[i], dataCounts[i], dataStrides[i],
//            rayOrigin, rayDirection,
//            (enum ObjectType) i);
//        
//        if (intersection.intersected
//         && intersection.timeOfIntersection < bestIntersection.timeOfIntersection) {
//            bestIntersection = intersection;
//        }
//    }
//    
//    return bestIntersection;
//}
//
/////
//global kernel void emit_photon(
//    ///
//    const unsigned int brdf, // one of (enum BRDFType)
//    
//    /// scene elements
//    __global float * sphereData,
//    const unsigned int numSpheres,
//
//    __global float * planeData,
//    const unsigned int numPlanes,
//
//    // light data
//    __global float * lightData,
//    const unsigned int numLights,
//    
//    const float luminosityPerPhoton, // lumens/(float)numRays
//    const float photonBounceProbability,
//    const float photonBounceEnergyMultipler
//    //
//    ) {
//    
//    struct SceneConfig config;
//    
//    config.brdf = (enum BRDFType) brdf;
//    config.sphereData = sphereData;
//    config.numSpheres = numSpheres;
//    config.planeData = planeData;
//    config.numPlanes = numPlanes;
//    
//    config.lightData = lightData;
//    config.numLights = numLights;
//    config.luminosityPerPhoton = luminosityPerPhoton;
//    config.photonBounceProbability = photonBounceProbability;
//    config.photonBounceEnergyMultipler = photonBounceEnergyMultipler;
//    
//    
//    int whichPhoton = (int) get_global_id(0);
//    
//    MWC64X_SeedStreams(&config.generator, (ulong) get_global_id(0), (ulong) get_global_size(0));
//    
//    /// choose a random light and direction
//    int whichLight = ((int) MWC64X_NextUint(&config.generator)) % numLights;
//    struct PovrayLightSourceData light = PovrayLightSourceData_fromData(whichLight * lightData[kPovrayLightSourceStride]);
//    
//    float u = ((float) (((int) MWC64X_NextUint(&config.generator)) % 100000)) / 100000.0f;
//    float v = ((float) (((int) MWC64X_NextUint(&config.generator)) % 100000)) / 100000.0f;
//    
//    float3 rayOrigin = light.position;
//    float3 rayDirection = cosineSampleSphere(u, v).xyz;
//    
//    /// Calculate intersections
//    struct RayIntersectionResult bestIntersection = SceneConfig_findClosestIntersection(&config, rayOrigin, rayDirection);
//    processHit(&config, light.color.xyz * luminosityPerPhoton, rayOrigin, rayDirection, bestIntersection);
//}
//
/////
//void processHits(
//    struct SceneConfig * config,
//
//    ///
//    RGBf energy,
//    float3 rayOrigin,
//    float3 rayDirection,
//    struct RayIntersectionResult hit) {
//    
//    /// bounce around the other photon
//    if (hit.intersected) {
//    
//        struct JensenPhoton photon;
//        
//        photon.position = RayIntersectionResult_locationOfIntersection(&hit);
//        photon.incomingDirection = rayDirection;
//        photon.energy = energy;
//        
//        photon.geometryType = hit.type;
//        photon.geometryDataPtr = hit.dataPtr;
//        
//        float value = ((float) (((int) MWC64X_NextUint(&config->generator)) % 100000)) / 100000.0f;
//        
//        if (value < photonBounceProbability) {
//            
//            float3 reflectedRay_direction = RayIntersectionResult_outgoingDirection(&hit);
//            float reflectedRay_origin = RayIntersectionResult_locationOfIntersection(&hit) + 0.001 * reflectedRay_direction;
//            
//            float3 hitEnergy = computeOutputEnergyForHit(brdf, hit, -rayDirection, reflectedRay_direction) * photonBounceEnergyMultipler;
//            
//            
//            /// Calculate intersections
//            struct RayIntersectionResult bestIntersection = SceneConfig_findClosestIntersection(config, reflectedRay_origin, reflectedRay_direction);
//            processHits(config, hitEnergy, reflectedRay_origin, reflectedRay_direction, bestIntersection);
//        }
//        else {
//            PhotonHashmap_setPhoton(&config->map, &photon, (int) get_global_id(0));
//        }
//    }
//}

#endif /* emit_photons_h */
