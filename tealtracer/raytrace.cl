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

////////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_sortPhotons
///
///
///
/// NOTE:  N threads, WG is workgroup size. Sort WG input blocks in each workgroup.
global kernel void photonmap_sortPhotons(void) {
//    __global const data_t * in,
//    __global data_t * out,
//    __local data_t * aux) {
  
//    int i = get_local_id(0); // index in workgroup
//    int wg = get_local_size(0); // workgroup size = block size, power of 2
//    
//    // Move IN, OUT to block start
//    int offset = get_group_id(0) * wg;
//    in += offset; out += offset;
//    
//    // Load block in AUX[WG]
//    aux[i] = in[i];
//    barrier(CLK_LOCAL_MEM_FENCE); // make sure AUX is entirely up to date
//    
//    // Now we will merge sub-sequences of length 1,2,...,WG/2
//    for (int length=1;length<wg;length<<=1)
//    {
//        data_t iData = aux[i];
//        uint iKey = getKey(iData);
//        int ii = i & (length-1);  // index in our sequence in 0..length-1
//        int sibling = (i - ii) ^ length; // beginning of the sibling sequence
//        int pos = 0;
//        for (int inc=length;inc>0;inc>>=1) // increment for dichotomic search
//        {
//            int j = sibling+pos+inc-1;
//            uint jKey = getKey(aux[j]);
//            bool smaller = (jKey < iKey) || ( jKey == iKey && j < i );
//            pos += (smaller)?inc:0;
//            pos = min(pos,length);
//        }
//        int bits = 2*length-1; // mask for destination
//        int dest = ((ii + pos) & bits) | (i & ~bits); // destination index in merged sequence
//        barrier(CLK_LOCAL_MEM_FENCE);
//        aux[dest] = iData;
//        barrier(CLK_LOCAL_MEM_FENCE);
//    }
//    
//    // Write output
//    out[i] = aux[i];
}

///////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_mapPhotonToGrid
///
/// SYNOPSIS: Called after sorting all of the photon data.
///
/// NOTE: Called over "photons.size()" photons
///

global kernel void photonmap_mapPhotonToGrid(
    // Grid specification
    PHOTON_HASHMAP_BASIC_PARAMS,
    PHOTON_HASHMAP_PHOTON_PARAMS,
    PHOTON_HASHMAP_META_PARAMS
    ) {

    struct PhotonHashmap map;
    PHOTON_HASHMAP_SET_BASIC_PARAMS();
    PHOTON_HASHMAP_SET_PHOTON_PARAMS();
    PHOTON_HASHMAP_SET_META_PARAMS();
    
    int index = (int) get_global_id(0);
    
    PhotonHashmap_mapPhotonToGrid(&map, index);
}

///////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_computeGridFirstPhoton
///
/// SYNOPSIS: Called after "mapPhotonToGrid" has been run on the hashmap data.
///
/// NOTE: Called over "photons.size()" photons
///

global kernel void photonmap_computeGridFirstPhoton(
    // Grid specification
    PHOTON_HASHMAP_META_PARAMS) {

    struct PhotonHashmap map;
    PHOTON_HASHMAP_SET_META_PARAMS();
    
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

global kernel void raytrace_one_ray(
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

    float3 forward = mat4x4_mult4x1(&viewTransform, (float4){Forward.x, Forward.y, Forward.z, 0.0}).xyz;
    float3 up = mat4x4_mult4x1(&viewTransform, (float4){Up.x, Up.y, Up.z, 0.0}).xyz * length(camera_up);
    float3 right = mat4x4_mult4x1(&viewTransform, (float4){Right.x, Right.y, Right.z, 0.0}).xyz * length(camera_right);
    
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
    
    /// This code is about a bit slower than the above, but is more generic and allows for
    /// incremental intersection search
//    struct IntersectionQuery query;
//    query.sphereData = sphereData;
//    query.numSphereDataElements = numSpheres;
//    query.sphereDataStride = kPovraySphereStride;
//    
//    query.planeData = planeData;
//    query.numPlaneDataElements = numPlanes;
//    query.planeDataStride = kPovrayPlaneStride;
//    
//    query.rayOrigin = rayOrigin;
//    query.rayDirection = rayDirection;
//    
//    struct RayIntersectionResult bestIntersection = IntersectionQuery_findClosestIntersection(&query);
    
    /// Calculate color
    if (bestIntersection.intersected) {
        RGBf energy = computeOutputEnergyHit((enum BRDFType) brdf, bestIntersection, (float3) {1,0,0}, -bestIntersection.rayDirection);
        color = ubyte4_make(energy.x * 255, energy.y * 255, energy.z * 255, 255);
    }
    
    /// Update the output
    for (int i = 0; i < 4; i++) {
        imageOutput[4 * (px + imageWidth * py) + i] = color.bytes[i];
    }
}

