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

#define PHOTON_HASHMAP_BASIC_PARAMS \
    const int map_spacing, \
    const float map_xmin, \
    const float map_ymin, \
    const float map_zmin, \
    const float map_xmax, \
    const float map_ymax, \
    const float map_zmax, \
    const int map_xdim, \
    const int map_ydim, \
    const int map_zdim, \
    const float map_cellsize, \
    const int map_numPhotons

#define PHOTON_HASHMAP_PHOTON_PARAMS \
    __global float * map_photon_floats, \
    __global float * __global * map_photon_geomptrs, \
    __global int * map_photon_objTypes

#define PHOTON_HASHMAP_META_PARAMS \
    __global int * map_gridIndices, \
    __global int * map_gridFirstPhotonIndices

#define PHOTON_HASHMAP_SET_BASIC_PARAMS() \
    map.spacing = map_spacing; \
    map.xmin = map_xmin; \
    map.ymin = map_ymin; \
    map.zmin = map_zmin; \
    map.xmax = map_xmax; \
    map.ymax = map_ymax; \
    map.zmax = map_zmax; \
    map.xdim = map_xdim; \
    map.ydim = map_ydim; \
    map.zdim = map_zdim; \
    map.cellsize = map_cellsize; \
    map.numPhotons =  map_numPhotons

#define PHOTON_HASHMAP_SET_PHOTON_PARAMS() \
    map.photon_floats = map_photon_floats; \
    map.photon_geomptrs = map_photon_geomptrs; \
    map.photon_objTypes = map_photon_objTypes

#define PHOTON_HASHMAP_SET_META_PARAMS() \
    map.gridIndices = map_gridIndices; \
    map.gridFirstPhotonIndices = map_gridFirstPhotonIndices

// TODO: Figure out a fast GPU algorithm for sorting the photons
global kernel void photonmap_sortPhotons(
    void
    ) {

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

