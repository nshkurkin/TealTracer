//
//  photon_tiling.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef photon_tiling_h
#define photon_tiling_h

#include "random.cl"
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

///
struct PhotonTiler {
    int tileWidth;
    int tileHeight;
    float photonEffectRadius;
    float photonSampleRate;
    const global float * tilePhotons; // contains a sum("photonCount")*sizeof(Photon)/sizeof(float) photons slots
    const global int * tilePhotonCount; // a "tiles_size" number of photon counts
    const global int * tilePhotonStarts; // points to the starts in "tilePhotons" (there are "tiles_size" of these)
};

///
int PhotonTiler_tileIndexForPixel(
    struct PhotonTiler * photonTiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py
);
///
const global float * PhotonTiler_getTilePhotonArrayInfoForPixel(
    struct PhotonTiler * photonTiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py,
    
    ///
    int * tilePhotonCount
);

///
RGBf PhotonTiler_computeOutputEnergyForHit(
    struct PhotonTiler * tiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py,
    
    enum BRDFType brdf,
    struct RayIntersectionResult * hitResult
);


///
int PhotonTiler_tileIndexForPixel(
    struct PhotonTiler * photonTiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py
) {
    return (px/photonTiler->tileWidth) + (py/photonTiler->tileHeight)*(imageWidth/photonTiler->tileWidth);
}

///
const global float * PhotonTiler_getTilePhotonArrayInfoForPixel(
    struct PhotonTiler * photonTiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py,
    
    ///
    int * tilePhotonCount
) {
    
    int tileIndex = PhotonTiler_tileIndexForPixel(photonTiler, imageWidth, imageHeight, px, py);
    
    *tilePhotonCount = photonTiler->tilePhotonCount[tileIndex];
    return &photonTiler->tilePhotons[photonTiler->tilePhotonStarts[tileIndex] * kJensenPhoton_floatStride];
}

///
RGBf PhotonTiler_computeOutputEnergyForHit(
    struct PhotonTiler * tiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py,
    
    enum BRDFType brdf,
    struct RayIntersectionResult * hitResult
) {

    RGBf output = (RGBf) {0,0,0};
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
    
    switch (hitResult->type) {
        case SphereObjectType: {
            struct PovraySphereData data = PovraySphereData_fromData(hitResult->dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        }
        case PlaneObjectType: {
            struct PovrayPlaneData data = PovrayPlaneData_fromData(hitResult->dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        };
        default: {
            break;
        }
    }
    
    int tileIndex = PhotonTiler_tileIndexForPixel(tiler, imageWidth, imageHeight, px, py);
    int photonCount = tiler->tilePhotonCount[tileIndex];
    const global float * photons = &tiler->tilePhotons[tiler->tilePhotonStarts[tileIndex] * kJensenPhoton_floatStride];

    float3 intersection = RayIntersectionResult_locationOfIntersection(hitResult);
    
    int i = 0;
    int numPhotonsSampled = 0;
    float maxDistanceSqd = -1.0f;
    
    /// Sample the collection of photons
    while (i < photonCount) {
        struct JensenPhoton photon = JensenPhoton_fromData(photons, i);
        float distanceSqrd = dot(photon.position - intersection, photon.position - intersection);
        if (hitResult->geomId < photon.geomId + 0.01f && hitResult->geomId > photon.geomId - 0.01f
         && distanceSqrd <= tiler->photonEffectRadius * tiler->photonEffectRadius) {
            ++numPhotonsSampled;
            output += computeOutputEnergyForBRDF(brdf, pigment, finish, photon.energy, -photon.incomingDirection, -hitResult->rayDirection, hitResult->surfaceNormal);
            maxDistanceSqd = max(maxDistanceSqd, distanceSqrd);
        }
        i++;
    }
    
    if (numPhotonsSampled > 0) {
        output = output * (float) (1.0f/(M_PI * maxDistanceSqd));
    }
    
    return output;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

///
struct PhotonTilerSingle {
    int tileX;
    int tileY;
    int tileWidth;
    int tileHeight;
    float photonEffectRadius;
    float photonSampleRate;
    const global float * tilePhotons; // contains a sum("photonCount")*sizeof(Photon)/sizeof(float) photons slots
    int tilePhotonCount; // a "tiles_size" number of photon counts
};

///
RGBf PhotonTilerSingle_computeOutputEnergyForHit(
    struct PhotonTilerSingle * tiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py,
    
    enum BRDFType brdf,
    struct RayIntersectionResult * hitResult
);

///
RGBf PhotonTilerSingle_computeOutputEnergyForHit(
    struct PhotonTilerSingle * tiler,
    const int imageWidth, const int imageHeight,
    const int px, const int py,
    
    enum BRDFType brdf,
    struct RayIntersectionResult * hitResult
) {

    RGBf output = (RGBf) {0,0,0};
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
    
    switch (hitResult->type) {
        case SphereObjectType: {
            struct PovraySphereData data = PovraySphereData_fromData(hitResult->dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        }
        case PlaneObjectType: {
            struct PovrayPlaneData data = PovrayPlaneData_fromData(hitResult->dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        };
        default: {
            break;
        }
    }
    
    int photonCount = tiler->tilePhotonCount;
    const global float * photons = tiler->tilePhotons;
    float3 intersection = RayIntersectionResult_locationOfIntersection(hitResult);
    
    int i = 0;
    int numPhotonsSampled = 0;
    float maxDistanceSqd = -1.0f;
    
    /// Sample the collection of photons
    while (i < photonCount) {
        struct JensenPhoton photon = JensenPhoton_fromData(photons, i);
        float distanceSqrd = dot(photon.position - intersection, photon.position - intersection);
        if (hitResult->geomId < photon.geomId + 0.01f && hitResult->geomId > photon.geomId - 0.01f
         && distanceSqrd <= tiler->photonEffectRadius * tiler->photonEffectRadius) {
            ++numPhotonsSampled;
            output += computeOutputEnergyForBRDF(brdf, pigment, finish, photon.energy, -photon.incomingDirection, -hitResult->rayDirection, hitResult->surfaceNormal);
            maxDistanceSqd = max(maxDistanceSqd, distanceSqrd);
        }
        i += tiler->photonSampleRate;
    }
    
    if (numPhotonsSampled > 0) {
        output = output * (float) (1.0f/(M_PI * maxDistanceSqd));
    }
    
    return output;
}


#endif /* photon_tiling_h */
