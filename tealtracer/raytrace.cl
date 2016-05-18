//
//  raytrace.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/7/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "matrix_math.cl"
#include "scene_objects.cl"
#include "intersection.cl"
#include "photon_hashmap.cl"
#include "coloring.cl"
#include "random.cl"

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
    
    // photon map data
    struct PhotonHashmap map;
    
    /// miscellaneous
    struct mwc64x_state_t generator;
};

///
struct RayIntersectionResult SceneConfig_findClosestIntersection(struct SceneConfig * config, float3 rayOrigin, float3 rayDirection);
void SceneConfig_initGenerator(struct SceneConfig * config, unsigned int generatorSeed);
float SceneConfig_randomNormalizedFloat(struct SceneConfig * config);
int SceneConfig_randomInt(struct SceneConfig * config);

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

///
void SceneConfig_initGenerator(struct SceneConfig * config, unsigned int generatorSeed) {
    MWC64X_SeedStreams(&(config->generator), (ulong) get_global_id(0) + generatorSeed, (ulong) get_global_size(0) + generatorSeed);
}

///
float SceneConfig_randomNormalizedFloat(struct SceneConfig * config) {
    return ((float) (MWC64X_NextUint(&(config->generator)) % 100000)) / 100000.0f;
}

///
int SceneConfig_randomInt(struct SceneConfig * config) {
    return (int) MWC64X_NextUint(&(config->generator));
}

///
void processEmittedPhoton(
    struct SceneConfig * config,
    int whichPhoton,
    
    ///
    RGBf sourceLightEnergy,
    float3 initialRayOrigin,
    float3 initialRayDirection,
    /// output
    bool * photonStored);

///
kernel void emit_photon(
    ///
    const unsigned int generatorSeed,
    const unsigned int brdf, // one of (enum BRDFType)
        
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
    PHOTON_HASHMAP_PHOTON_PARAMS
    ) {
    
    int whichPhoton = (int) get_global_id(0);
    if (whichPhoton >= map_numPhotons) {
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
    
    PHOTON_HASHMAP_SET_PHOTON_PARAMS((&config.map));
    
    SceneConfig_initGenerator(&config, generatorSeed);
    
    /// choose a random light and direction
    int whichLight = SceneConfig_randomInt(&config) % numLights;
    struct PovrayLightSourceData light = PovrayLightSourceData_fromData(&lightData[kPovrayLightSourceStride * whichLight]);
    
    bool photonStored = false;
    
    while (!photonStored) {
        float u = SceneConfig_randomNormalizedFloat(&config), v = SceneConfig_randomNormalizedFloat(&config);
        
        float3 rayOrigin = light.position;
        float3 rayDirection = uniformSampleSphere(u, v).xyz;
        
        processEmittedPhoton(&config, whichPhoton, light.color.xyz * luminosityPerPhoton, rayOrigin, rayDirection, &photonStored);
    }
}

void processEmittedPhoton(
    struct SceneConfig * config,
    
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
            PhotonHashmap_setPhoton(&config->map, &photon, whichPhoton);
            *photonStored = true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_mapPhotonToGrid
///
/// SYNOPSIS: Called after sorting all of the photon data.
///
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

///////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_initGridFirstPhoton
///
/// SYNOPSIS: Called after "mapPhotonToGrid" has been run on the hashmap data.
///
/// NOTE: Called over "map->xdim * map->ydim * map->zdim"
///
kernel void photonmap_initGridFirstPhoton(
    const int map_xdim,
    const int map_ydim,
    const int map_zdim,
    __global int * map_gridFirstPhotonIndices) {
    
    int index = (int) get_global_id(0);
    
    if (index > map_xdim * map_ydim * map_zdim) {
        return;
    }
    
    map_gridFirstPhotonIndices[index] = -1;
}

///////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_computeGridFirstPhoton
///
/// SYNOPSIS: Called after "mapPhotonToGrid" has been run on the hashmap data.
///
/// NOTE: Called over "photons.size()" photons
///

kernel void photonmap_computeGridFirstPhoton(
    // Grid specification
    PHOTON_HASHMAP_PHOTON_PARAMS,
    PHOTON_HASHMAP_META_PARAMS) {

    struct PhotonHashmap map;
    
    PHOTON_HASHMAP_SET_PHOTON_PARAMS((&map));
    PHOTON_HASHMAP_SET_META_PARAMS((&map));
    
    int index = (int) get_global_id(0);
    
    PhotonHashmap_computeGridFirstPhoton(&map, index);
}

////////////////////////////////////////////////////////////////////////////
///
/// KERNEL: raytrace_one_ray
///
/// SYNOPSIS: Used to cast a single ray into the scene. It calculates the ray
///     based on the global thread ID. Rays are then intersected with elements
///     in the scene and the closest found object will be used as the color
///     sample source. The resulting color is placed in the output texture buffer.
///

kernel void raytrace_one_ray(
    /// input
    const float3 camera_location,
    const float3 camera_up,
    const float3 camera_right,
    const float3 camera_lookAt,
    
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
    
    ///
    struct PhotonHashmap map;
    
    PHOTON_HASHMAP_SET_BASIC_PARAMS((&map));
    PHOTON_HASHMAP_SET_PHOTON_PARAMS((&map));
    PHOTON_HASHMAP_SET_META_PARAMS((&map));
    
    /// Create the ray for this given pixel
    struct mat4x4 viewTransform;

    mat4x4_loadLookAt(&viewTransform, camera_location, camera_lookAt, camera_up);

    float3 forward = mat4x4_mult4x1(&viewTransform, (float4){Forward.x, Forward.y, Forward.z, 0.0f}).xyz;
    float3 up = mat4x4_mult4x1(&viewTransform, (float4){Up.x, Up.y, Up.z, 0.0f}).xyz * length(camera_up);
    float3 right = mat4x4_mult4x1(&viewTransform, (float4){Right.x, Right.y, Right.z, 0.0f}).xyz * length(camera_right);
    
    unsigned int threadId = (unsigned int) get_global_id(0);
    
    if (threadId >= imageWidth * imageHeight) {
        return;
    }
    
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    
    float3 rayOrigin = camera_location;
    float3 rayDirection = normalize(forward - 0.5f*up - 0.5f*right
        + right * ((0.5f+(float)px)/(float)imageWidth)
        + up * ((0.5f+(float)py)/(float)imageHeight));
    
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

////////////////////////////////////////////////////////////////////////////
///
/// KERNEL: raytrace_one_ray_direct
///
/// SYNOPSIS: Used to cast a single ray into the scene. It calculates the ray
///     based on the global thread ID. Rays are then intersected with elements
///     in the scene and the closest found object will be used as the color
///     sample source. The resulting color is placed in the output texture buffer.
///

kernel void raytrace_one_ray_direct(
    /// input
    const float3 camera_location,
    const float3 camera_up,
    const float3 camera_right,
    const float3 camera_lookAt,
    
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
    
    /// Create the ray for this given pixel
    struct mat4x4 viewTransform;

    mat4x4_loadLookAt(&viewTransform, camera_location, camera_lookAt, camera_up);

    float3 forward = mat4x4_mult4x1(&viewTransform, (float4){Forward.x, Forward.y, Forward.z, 0.0f}).xyz;
    float3 up = mat4x4_mult4x1(&viewTransform, (float4){Up.x, Up.y, Up.z, 0.0f}).xyz * length(camera_up);
    float3 right = mat4x4_mult4x1(&viewTransform, (float4){Right.x, Right.y, Right.z, 0.0f}).xyz * length(camera_right);
    
    unsigned int threadId = (unsigned int) get_global_id(0);
    
    if (threadId >= imageWidth * imageHeight) {
        return;
    }
    
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    
    float3 rayOrigin = camera_location;
    float3 rayDirection = normalize(forward - 0.5f*up - 0.5f*right
        + right * ((0.5f+(float)px)/(float)imageWidth)
        + up * ((0.5f+(float)py)/(float)imageHeight));
    
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

////////////////////////////////////////////////////////////////////////////
///
/// KERNEL: raytrace_one_ray_tiled
///
/// SYNOPSIS: Used to cast a single ray into the scene. It calculates the ray
///     based on the global thread ID. Rays are then intersected with elements
///     in the scene and the closest found object will be used as the color
///     sample source. The resulting color is placed in the output texture buffer.
///

//kernel void raytrace_one_ray_tiled(
//    /// input
//    const float3 camera_location,
//    const float3 camera_up,
//    const float3 camera_right,
//    const float3 camera_lookAt,
//    
//    const unsigned int brdf, // one of (enum BRDFType)
//    
//    __global float * sphereData,
//    const unsigned int numSpheres,
//
//    __global float * planeData,
//    const unsigned int numPlanes,
//    
//    __global float * lightData,
//    const unsigned int numLights,
//    
//    /// output
//    __write_only image2d_t image_output,
//    const unsigned int imageWidth,
//    const unsigned int imageHeight
//    ) {
//    
//    /// Create the ray for this given pixel
//    struct mat4x4 viewTransform;
//
//    mat4x4_loadLookAt(&viewTransform, camera_location, camera_lookAt, camera_up);
//
//    float3 forward = mat4x4_mult4x1(&viewTransform, (float4){Forward.x, Forward.y, Forward.z, 0.0f}).xyz;
//    float3 up = mat4x4_mult4x1(&viewTransform, (float4){Up.x, Up.y, Up.z, 0.0f}).xyz * length(camera_up);
//    float3 right = mat4x4_mult4x1(&viewTransform, (float4){Right.x, Right.y, Right.z, 0.0f}).xyz * length(camera_right);
//    
//    unsigned int threadId = (unsigned int) get_global_id(0);
//    
//    if (threadId >= imageWidth * imageHeight) {
//        return;
//    }
//    
//    int px = threadId % imageWidth;
//    int py = threadId / imageWidth;
//    
//    float3 rayOrigin = camera_location;
//    float3 rayDirection = normalize(forward - 0.5f*up - 0.5f*right
//        + right * ((0.5f+(float)px)/(float)imageWidth)
//        + up * ((0.5f+(float)py)/(float)imageHeight));
//    
//    struct SceneConfig scene;
//    scene.brdf = brdf;
//    scene.sphereData = sphereData;
//    scene.numSpheres = numSpheres;
//    scene.planeData = planeData;
//    scene.numPlanes = numPlanes;
//    scene.lightData = lightData;
//    scene.numLights = numLights;
//    
//    struct RayIntersectionResult bestIntersection = SceneConfig_findClosestIntersection(&scene, rayOrigin, rayDirection);
//    
//    RGBf energy = (RGBf) {0, 0, 0};
//    /// Calculate color
//    if (bestIntersection.intersected) {
//    
//        /// Get direct lighting
//        for (unsigned int lightItr = 0; lightItr < scene.numLights; lightItr++) {
//            
//            struct PovrayLightSourceData light = PovrayLightSourceData_fromData(&scene.lightData[kPovrayLightSourceStride * lightItr]);
//            float3 hitLoc = RayIntersectionResult_locationOfIntersection(&bestIntersection);
//            float3 toLight = light.position - hitLoc;
//            float3 toLightDir = normalize(toLight);
//            
//            float3 shadowRay_origin = hitLoc + 0.01f * toLightDir;
//            float3 shadowRay_direction = toLightDir;
//            
//            struct RayIntersectionResult shadowHitTest = SceneConfig_findClosestIntersection(&scene, shadowRay_origin, shadowRay_direction);
//                                        
//            bool isShadowed = !(!shadowHitTest.intersected
//             || (shadowHitTest.intersected && shadowHitTest.timeOfIntersection > length(toLight)));
//                    
//            if (!isShadowed) {
//                energy += computeOutputEnergyForHit(brdf, bestIntersection, light.color.xyz, toLightDir, normalize(camera_location - hitLoc));
//            }
//        }
//    }
//    
//    write_imagef(image_output, (int2) {px, py}, (float4) {
//        min(energy.x, 1.0f),
//        min(energy.y, 1.0f),
//        min(energy.z, 1.0f),
//        1.0f
//    });
//}


