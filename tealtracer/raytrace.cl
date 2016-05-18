//
//  raytrace.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/7/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "scene_config.cl"
#include "photon_hashmap.cl"

/// NOTE: called over "numPhotons"
///
kernel void emit_photon(
    ///
    const unsigned int generatorSeed,
    const unsigned int brdf, // one of (enum BRDFType)
    
    const int _arg_buffer_, // for some reason mis-aligned data causes weird errors?
    
    /// scene elements
    __global float * sphereData,
    const unsigned int numSpheres,

    __global float * planeData,
    const unsigned int numPlanes,

    // light data
    __global float * lightData,
    const unsigned int numLights,
    
    const float luminosityPerPhoton, // lumens/(float)numRays
    const float photonBounceProbability,
    const float photonBounceEnergyMultipler,
    
    ///
    global float * photons, // "numPhotons" photons = numPhotons * sizeof(JensenPhoton)/sizeof(float)
    const int numPhotons
    ) {
    
    int whichPhoton = (int) get_global_id(0);
    if (whichPhoton >= numPhotons) {
        return;
    }
    
    struct SceneConfig config;
    
    config.brdf = (enum BRDFType) brdf;
    config.sphereData = sphereData;
    config.numSpheres = numSpheres;
    config.planeData = planeData;
    config.numPlanes = numPlanes;
    
    config.lightData = lightData;
    config.numLights = numLights;
    config.luminosityPerPhoton = luminosityPerPhoton;
    config.photonBounceProbability = photonBounceProbability;
    config.photonBounceEnergyMultipler = photonBounceEnergyMultipler;
    
    SceneConfig_initGenerator(&config, generatorSeed);
    
    /// choose a random light and direction
    int whichLight = SceneConfig_randomInt(&config) % numLights;
    struct PovrayLightSourceData light = PovrayLightSourceData_fromData(&lightData[kPovrayLightSourceStride * whichLight]);
    
    bool photonStored = false;
    
    while (!photonStored) {
        float u = SceneConfig_randomNormalizedFloat(&config), v = SceneConfig_randomNormalizedFloat(&config);
        
        float3 rayOrigin = light.position;
        float3 rayDirection = uniformSampleSphere(u, v).xyz;
        
        processEmittedPhoton(&config, photons, whichPhoton, light.color.xyz * luminosityPerPhoton, rayOrigin, rayDirection, &photonStored);
    }
}

/// NOTE: called over imageWidth * imageHeight pixels
///
kernel void raytrace_one_ray_direct(
    /// input
    const float3 camera_location,
    const float3 camera_up,
    const float3 camera_right,
    const float3 camera_forward,
    
    const unsigned int brdf, // one of (enum BRDFType)
    
    __global float * sphereData,
    const unsigned int numSpheres,

    __global float * planeData,
    const unsigned int numPlanes,
    
    __global float * lightData,
    const unsigned int numLights,
    
    /// output
    __write_only image2d_t image_output,
    const unsigned int imageWidth,
    const unsigned int imageHeight
    ) {
    
    unsigned int threadId = (unsigned int) get_global_id(0);
    if (threadId >= imageWidth * imageHeight) {
        return;
    }
       
    /// Create the ray for this given pixel
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    
    float3 rayOrigin = camera_location;
    float3 rayDirection = normalize(camera_forward - 0.5f*camera_up - 0.5f*camera_right
        + camera_right * ((0.5f+(float)px)/(float)imageWidth)
        + camera_up * ((0.5f+(float)py)/(float)imageHeight));
    
    struct SceneConfig scene;
    scene.brdf = brdf;
    scene.sphereData = sphereData;
    scene.numSpheres = numSpheres;
    scene.planeData = planeData;
    scene.numPlanes = numPlanes;
    scene.lightData = lightData;
    scene.numLights = numLights;
    
    struct RayIntersectionResult bestIntersection = SceneConfig_findClosestIntersection(&scene, rayOrigin, rayDirection);
    
    RGBf energy = (RGBf) {0, 0, 0};
    /// Calculate color
    if (bestIntersection.intersected) {
    
        /// Get direct lighting
        for (unsigned int lightItr = 0; lightItr < scene.numLights; lightItr++) {
            
            struct PovrayLightSourceData light = PovrayLightSourceData_fromData(&scene.lightData[kPovrayLightSourceStride * lightItr]);
            float3 hitLoc = RayIntersectionResult_locationOfIntersection(&bestIntersection);
            float3 toLight = light.position - hitLoc;
            float3 toLightDir = normalize(toLight);
            
            float3 shadowRay_origin = hitLoc + 0.01f * toLightDir;
            float3 shadowRay_direction = toLightDir;
            
            struct RayIntersectionResult shadowHitTest = SceneConfig_findClosestIntersection(&scene, shadowRay_origin, shadowRay_direction);
                                        
            bool isShadowed = !(!shadowHitTest.intersected
             || (shadowHitTest.intersected && shadowHitTest.timeOfIntersection > length(toLight)));
                    
            if (!isShadowed) {
                energy += computeOutputEnergyForHit(brdf, bestIntersection, light.color.xyz, toLightDir, normalize(camera_location - hitLoc));
            }
        }
    }
    
    write_imagef(image_output, (int2) {px, py}, (float4) {
        min(energy.x, 1.0f),
        min(energy.y, 1.0f),
        min(energy.z, 1.0f),
        1.0f
    });
}

/// SYNOPSIS: Called after sorting all of the photon data.
/// NOTE: Called over "photons.size()" photons
///
kernel void photonmap_mapPhotonToGrid(
    // Grid specification
    PHOTON_HASHMAP_BASIC_PARAMS,
    PHOTON_HASHMAP_PHOTON_PARAMS,
    PHOTON_HASHMAP_META_PARAMS
) {

    struct PhotonHashmap map;
    PHOTON_HASHMAP_SET_BASIC_PARAMS((&map));
    PHOTON_HASHMAP_SET_PHOTON_PARAMS((&map));
    PHOTON_HASHMAP_SET_META_PARAMS((&map));
    
    int index = (int) get_global_id(0);
    if (index >= map.numPhotons) {
        return;
    }
    
    struct JensenPhoton photon = JensenPhoton_fromData(map.photon_data, index);
    map.gridIndices[index] = PhotonHashmap_clampedCellIndexHash(&map, photon.position);
}

/// SYNOPSIS: Called after "mapPhotonToGrid" has been run on the hashmap data.
/// NOTE: Called over "map->xdim * map->ydim * map->zdim"
///
kernel void photonmap_initGridFirstPhoton(
    const int map_xdim,
    const int map_ydim,
    const int map_zdim,
    global int * map_gridFirstPhotonIndices
) {
    
    int index = (int) get_global_id(0);
    
    if (index > map_xdim * map_ydim * map_zdim) {
        return;
    }
    
    map_gridFirstPhotonIndices[index] = -1;
}

/// SYNOPSIS: Called after "mapPhotonToGrid" has been run on the hashmap data.
/// NOTE: Called over "photons.size()" photons
///
kernel void photonmap_computeGridFirstPhoton(
    // Grid specification
    PHOTON_HASHMAP_PHOTON_PARAMS,
    PHOTON_HASHMAP_META_PARAMS
) {

    struct PhotonHashmap map;
    
    PHOTON_HASHMAP_SET_PHOTON_PARAMS((&map));
    PHOTON_HASHMAP_SET_META_PARAMS((&map));
    
    int index = (int) get_global_id(0);
    
    PhotonHashmap_computeGridFirstPhoton(&map, index);
}

/// NOTE: called over imageWidth * imageHeight pixels
///
kernel void raytrace_one_ray_hashgrid(
    /// input
    const float3 camera_location,
    const float3 camera_up,
    const float3 camera_right,
    const float3 camera_forward,
    
    const unsigned int brdf, // one of (enum BRDFType)
    
    __global float * sphereData,
    const unsigned int numSpheres,

    __global float * planeData,
    const unsigned int numPlanes,
    
    __global float * lightData,
    const unsigned int numLights,
    
    /// usable data
    __global float * photon_index_array,
    const int maxNumPhotonsToGather,
    const float maxPhotonGatherDistance,

    /// photon map
    PHOTON_HASHMAP_BASIC_PARAMS,
    PHOTON_HASHMAP_PHOTON_PARAMS,
    PHOTON_HASHMAP_META_PARAMS,
    
    /// output
    __write_only image2d_t image_output,
    const unsigned int imageWidth,
    const unsigned int imageHeight
    ) {
    
    unsigned int threadId = (unsigned int) get_global_id(0);
    if (threadId >= imageWidth * imageHeight) {
        return;
    }
    
    ///
    struct PhotonHashmap map;
    
    PHOTON_HASHMAP_SET_BASIC_PARAMS((&map));
    PHOTON_HASHMAP_SET_PHOTON_PARAMS((&map));
    PHOTON_HASHMAP_SET_META_PARAMS((&map));
    
    /// Create the ray for this given pixel
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    
    float3 rayOrigin = camera_location;
    float3 rayDirection = normalize(camera_forward - 0.5f*camera_up - 0.5f*camera_right
        + camera_right * ((0.5f+(float)px)/(float)imageWidth)
        + camera_up * ((0.5f+(float)py)/(float)imageHeight));
    
    struct SceneConfig scene;
    scene.brdf = brdf;
    scene.sphereData = sphereData;
    scene.numSpheres = numSpheres;
    scene.planeData = planeData;
    scene.numPlanes = numPlanes;
    scene.lightData = lightData;
    scene.numLights = numLights;
    
    struct RayIntersectionResult bestIntersection = SceneConfig_findClosestIntersection(&scene, rayOrigin, rayDirection);
    
    RGBf energy = (RGBf) {0, 0, 0};
    /// Calculate color
    if (bestIntersection.intersected) {
        __global float * photon_indices = &photon_index_array[threadId * maxNumPhotonsToGather];
        energy = computeOutputEnergyForHitWithPhotonMap(brdf, bestIntersection, &map, maxNumPhotonsToGather, maxPhotonGatherDistance, -bestIntersection.rayDirection, photon_indices);
    }
    
    write_imagef(image_output, (int2) {px, py}, (float4) {
        min(energy.x, 1.0f),
        min(energy.y, 1.0f),
        min(energy.z, 1.0f),
        1.0f
    });
}
