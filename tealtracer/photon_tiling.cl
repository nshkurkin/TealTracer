//
//  photon_tiling.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef photon_tiling_h
#define photon_tiling_h

#include "photons.cl"

/// Data structures

struct Plane {
    float3 normal;
    float distance;
};
__constant const unsigned int kPlane_floatStride = 4;

__constant const unsigned int kFrustum_numPlanes = 4;
struct Frustum {
    struct Plane planes[kFrustum_numPlanes];
};
__constant const unsigned int kFrustum_floatStride = kPlane_floatStride * kFrustum_numPlanes;

struct Tile {
    struct Frustum frustum;
};
__constant const unsigned int kTile_floatStride = kFrustum_floatStride;

/// Data operations
///
///

void Tile_fromData(
    struct Tile * tile,
    global const float * data,
    size_t index
);
bool Frustum_intersectsOrContainsSphere(
    struct Frustum * frustum,
    float3 sphereCenter,
    float sphereRadius
);
float Plane_distanceToPoint(
    struct Plane * plane,
    float3 point
);

///
void Tile_fromData(struct Tile * tile, global const float * data, size_t index) {
    size_t dataIndex = index * kTile_floatStride;
    global const float * dataStart = &data[dataIndex];
   
    for (size_t planeItr = 0; planeItr < kFrustum_numPlanes; planeItr++) {
        size_t planeDataIndex = planeItr * kPlane_floatStride;
        
        tile->frustum.planes[planeItr].normal = (float3) {
            dataStart[planeDataIndex + 0],
            dataStart[planeDataIndex + 1],
            dataStart[planeDataIndex + 2]
        };
        tile->frustum.planes[planeItr].distance = dataStart[planeDataIndex + 3];
    }
}

///
bool Frustum_intersectsOrContainsSphere(
    struct Frustum * frustum,
    float3 sphereCenter,
    float sphereRadius
) {

    bool sphereCompletelyOutside = false;
    for (size_t planeItr = 0; planeItr < kFrustum_numPlanes && !sphereCompletelyOutside; planeItr++) {
        struct Plane * plane = &frustum->planes[planeItr];
        float distance = Plane_distanceToPoint(plane, sphereCenter);
        sphereCompletelyOutside = (distance + sphereRadius < 0);
    }
    
    return !sphereCompletelyOutside;
}

///
float Plane_distanceToPoint(
    struct Plane * plane,
    float3 point
) {
    return dot(plane->normal, point - plane->distance * plane->normal);
}

/// NOTE: Instanced over every photon
///
kernel void countPhotonsInTile(
    global const float * photons,
    const int photons_size,
    
    global const float * tiles,
    const int tiles_size,
    
    const float photonEffectRadius,
    
    volatile global int * photonCount // a "tiles_size" number of photon counts
) {

    unsigned int threadId = (unsigned int) get_global_id(0);
    if (threadId >= photons_size) {
        return;
    }

    size_t photonIdx = threadId;
    struct JensenPhoton photon = JensenPhoton_fromData(photons, photonIdx);

    for (size_t tileItr = 0; tileItr < tiles_size; tileItr++) {
        struct Tile tile;
        Tile_fromData(&tile, tiles, tileItr);
    
        if (Frustum_intersectsOrContainsSphere(&tile.frustum, photon.position, photonEffectRadius)) {
            atomic_add(photonCount + tileItr, 1);
        }
    }
}

/// NOTE: Instanced over every photon
///
kernel void copyPhotonsIntoTile(
    global const float * photons,
    const int photons_size,
    
    global const float * tiles,
    const int tiles_size,
    
    const float photonEffectRadius,
    const global int * photonCount, // a "tiles_size" number of photon counts
    
    volatile global int * nextPhotonIndex,
    global float * tilePhotons, // contains a sum("photonCount")*sizeof(Photon)/sizeof(float) photons slots
    const global int * tilePhotonStarts // points to the starts in "tilePhotons" (there are "tiles_size" of these)
) {

    unsigned int threadId = (unsigned int) get_global_id(0);
    if (threadId >= photons_size) {
        return;
    }

    size_t photonIdx = threadId;
    struct JensenPhoton photon = JensenPhoton_fromData(photons, photonIdx);

    for (size_t tileItr = 0; tileItr < tiles_size; tileItr++) {
        struct Tile tile;
        Tile_fromData(&tile, tiles, tileItr);
    
        if (Frustum_intersectsOrContainsSphere(&tile.frustum, photon.position, photonEffectRadius)) {
            int tilePhotonIdx = atomic_add(nextPhotonIndex + tileItr, 1);
            JensenPhoton_setData(&photon, &tilePhotons[tilePhotonStarts[tileItr]], tilePhotonIdx);
        }
    }
}

#endif /* photon_tiling_h */
