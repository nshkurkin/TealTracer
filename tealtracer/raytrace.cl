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
void SceneConfig_initGenerator(struct SceneConfig * config);
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
void SceneConfig_initGenerator(struct SceneConfig * config) {
    MWC64X_SeedStreams(&(config->generator), (ulong) get_global_id(0), (ulong) get_global_size(0));
}

///
float SceneConfig_randomNormalizedFloat(struct SceneConfig * config) {
    return ((float) (((int) MWC64X_NextUint(&(config->generator))) % 100000)) / 100000.0f;
}

///
int SceneConfig_randomInt(struct SceneConfig * config) {
    return (int) MWC64X_NextUint(&(config->generator));
}

///
void processEmittedPhoton(
    struct SceneConfig * config,

    ///
    RGBf sourceLightEnergy,
    float3 initialRayOrigin,
    float3 initialRayDirection,
    /// output
    bool * photonStored);

///
kernel void emit_photon(
    ///
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
    
    SceneConfig_initGenerator(&config);
    
    /// choose a random light and direction
    int whichLight = SceneConfig_randomInt(&config) % numLights;
    struct PovrayLightSourceData light = PovrayLightSourceData_fromData(&lightData[kPovrayLightSourceStride * whichLight]);
    
    bool photonStored = false;
    
    while (!photonStored) {
        float u = SceneConfig_randomNormalizedFloat(&config);
        float v = SceneConfig_randomNormalizedFloat(&config);
        
        float3 rayOrigin = light.position;
        float3 rayDirection = cosineSampleSphere(u, v).xyz;
        
        processEmittedPhoton(&config, light.color.xyz * luminosityPerPhoton, rayOrigin, rayDirection, &photonStored);
    }
}

void processEmittedPhoton(
    struct SceneConfig * config,
    
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
            
            
            /// NOTE: Super frustrating on the GPU; the following line crashes
            ///         clBuildProgram for whatever reason not explained! It has
            ///         instead been expanded below and it works?
            
//            RGBf hitEnergy = computeOutputEnergyForHit(config->brdf, hit, -rayDirection, reflectedRay_direction) * config->photonBounceEnergyMultipler;
            
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
    
            RGBf hitEnergy = computeOutputEnergyForBRDF(config->brdf, pigment, finish, (RGBf) {1.0f,1.0f,1.0f}, -rayDirection, reflectedRay_direction, hit.surfaceNormal) * config->photonBounceEnergyMultipler;
            //////
            
            /// Calculate intersection
            hit = SceneConfig_findClosestIntersection(config, reflectedRay_origin, reflectedRay_direction);
            rayOrigin = reflectedRay_origin;
            rayDirection = reflectedRay_direction;
            energy = hitEnergy;
        }
        else {
            PhotonHashmap_setPhoton(&config->map, &photon, (int) get_global_id(0));
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
    if (index < map.numPhotons) {
        PhotonHashmap_mapPhotonToGrid(&map, index);
    }
}

////////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_sortPhotonHash
///
/// SYNOPSIS: called after photons have been emitted and "photonmap_mapPhotonToGrid"
///             has completed.
///
/// NOTE:  N threads, WG is workgroup size. Sort WG input blocks in each workgroup.
///         Taken from: http://www.bealto.com/gpu-sorting_parallel-merge-local.html

kernel void photonmap_sortPhotonHash(
    __global const int * photon_hash_in,
    __global int * photon_hash_out,
    __local int * aux) {
  
    int i = get_local_id(0); // index in workgroup
    int wg = get_local_size(0); // workgroup size = block size, power of 2
    
    // Move IN, OUT to block start
    int offset = get_group_id(0) * wg;
    photon_hash_in += offset;
    photon_hash_out += offset;
    
    // Load block in AUX[WG]
    aux[i] = photon_hash_in[i];
    barrier(CLK_LOCAL_MEM_FENCE); // make sure AUX is entirely up to date
    
    // Now we will merge sub-sequences of length 1,2,...,WG/2
    for (int length = 1; length < wg; length <<= 1) {
        int iData = aux[i];
        uint iKey = iData;// getKey(iData);
        int ii = i & (length-1);  // index in our sequence in 0..length-1
        int sibling = (i - ii) ^ length; // beginning of the sibling sequence
        int pos = 0;
        // increment for dichotomic search
        for (int inc = length; inc > 0; inc >>= 1) {
            int j = sibling + pos + inc - 1;
            uint jKey = aux[j]; //getKey(aux[j]);
            bool smaller = (jKey < iKey) || ( jKey == iKey && j < i );
            pos += (smaller)? inc : 0;
            pos = min(pos,length);
        }
        
        int bits = 2*length - 1; // mask for destination
        int dest = ((ii + pos) & bits) | (i & ~bits); // destination index in merged sequence
        barrier(CLK_LOCAL_MEM_FENCE);
        aux[dest] = iData;
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    // Write output
    photon_hash_out[i] = aux[i];
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
    PHOTON_HASHMAP_META_PARAMS) {

    struct PhotonHashmap map;
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
    
    /// output
    /// image data is organized into 4 byte pixels in a height*width pixels
    __global unsigned char * imageOutput,
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
    
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    struct ubyte4 color;
    
    color = ubyte4_make(0,0,0,255);
    
    float3 rayOrigin
        = camera_location + forward - 0.5f*up - 0.5f*right
        + right * ((0.5f+(float)px)/(float)imageWidth)
        + up * ((0.5f+(float)py)/(float)imageHeight);
    float3 rayDirection = normalize(rayOrigin - camera_location);
    
    /// Calculate intersections
    struct RayIntersectionResult bestIntersection;
    bestIntersection.intersected = false;
    bestIntersection.timeOfIntersection = INFINITY;
    
    __global float * dataPtrs[2] = { sphereData, planeData };
    unsigned int dataCounts[2] = { numSpheres, numPlanes };
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
    
    /// Calculate color
    if (bestIntersection.intersected) {
        RGBf energy = computeOutputEnergyForHit(brdf, bestIntersection, (float3) {1,0,0}, -bestIntersection.rayDirection);
        color = ubyte4_make(energy.x * 255, energy.y * 255, energy.z * 255, 255);
    }
    
    /// Update the output
    for (int i = 0; i < 4; i++) {
        imageOutput[4 * (px + imageWidth * py) + i] = color.bytes[i];
    }
}

