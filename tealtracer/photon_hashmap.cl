//
//  photon_hashmap.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef photon_hashmap_h
#define photon_hashmap_h

#include "photons.cl"
    
//////////////////////////////////////////////////////////////////////////////
///
///
struct PhotonHashmap {
    int spacing;
    float xmin, ymin, zmin;
    float xmax, ymax, zmax;
    int xdim, ydim, zdim;
    float cellsize;
    
    /// Photon heap data
    __global float * photon_data;
    int numPhotons;
    
    /// Photon metadata
    __global int * gridIndices; // size: numPhotons
    __global int * gridFirstPhotonIndices; // size: xdim * ydim * zdim
};

int PhotonHashmap_photonHashIJK(struct PhotonHashmap * map, int i, int j, int k);
int PhotonHashmap_photonHash3i(struct PhotonHashmap * map, int3 index);
int PhotonHashmap_cellIndexHash(struct PhotonHashmap * map, float3 position);
int PhotonHashmap_clampedCellIndexHash(struct PhotonHashmap * map, float3 position);
int3 PhotonHashmap_cellIndex(struct PhotonHashmap * map, float3 position);

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
    const float map_cellsize

#define PHOTON_HASHMAP_PHOTON_PARAMS \
    __global float * map_photon_data, \
    const int map_numPhotons

#define PHOTON_HASHMAP_META_PARAMS \
    __global int * map_gridIndices, \
    __global int * map_gridFirstPhotonIndices

#define PHOTON_HASHMAP_SET_BASIC_PARAMS(map) \
    map->spacing = map_spacing; \
    map->xmin = map_xmin; \
    map->ymin = map_ymin; \
    map->zmin = map_zmin; \
    map->xmax = map_xmax; \
    map->ymax = map_ymax; \
    map->zmax = map_zmax; \
    map->xdim = map_xdim; \
    map->ydim = map_ydim; \
    map->zdim = map_zdim; \
    map->cellsize = map_cellsize

#define PHOTON_HASHMAP_SET_PHOTON_PARAMS(map) \
    map->photon_data = map_photon_data; \
    map->numPhotons =  map_numPhotons

#define PHOTON_HASHMAP_SET_META_PARAMS(map) \
    map->gridIndices = map_gridIndices; \
    map->gridFirstPhotonIndices = map_gridFirstPhotonIndices

///
int PhotonHashmap_photonHashIJK(struct PhotonHashmap * map, int i, int j, int k) {
    return i + (j * map->xdim) + (k * map->xdim * map->ydim);
}

///
int PhotonHashmap_photonHash3i(struct PhotonHashmap * map, int3 index) {
    return PhotonHashmap_photonHashIJK(map, index.x, index.y, index.z);
}

///
int PhotonHashmap_cellIndexHash(struct PhotonHashmap * map, float3 position) {
    return PhotonHashmap_photonHash3i(map, PhotonHashmap_cellIndex(map, position));
}

///
int3 PhotonHashmap_cellIndex(struct PhotonHashmap * map, float3 position) {
    int3 index;
    
    index.x = floor((position.x - map->xmin)/map->cellsize);
    index.y = floor((position.y - map->ymin)/map->cellsize);
    index.z = floor((position.z - map->zmin)/map->cellsize);
    
    return index;
}

///
int PhotonHashmap_clampedCellIndexHash(struct PhotonHashmap * map, float3 position) {
    int hash;
    int3 cellIndex = PhotonHashmap_cellIndex(map, position);
    if (cellIndex.x >= 0 && cellIndex.x < map->xdim
     && cellIndex.y >= 0 && cellIndex.y < map->ydim
     && cellIndex.z >= 0 && cellIndex.z < map->zdim) {
        hash = PhotonHashmap_photonHash3i(map, cellIndex);
    }
    else {
        hash = -1;
    }
    return hash;
}

///
void PhotonHashmap_computeGridFirstPhoton(struct PhotonHashmap * map, int index);
struct JensenPhoton PhotonHashmap_getPhoton(struct PhotonHashmap * map, int index);
void PhotonHashmap_setPhoton(struct PhotonHashmap * map, struct JensenPhoton * photon, int index);

///
struct JensenPhoton PhotonHashmap_getPhoton(struct PhotonHashmap * map, int index) {
    return JensenPhoton_fromData(map->photon_data, index);
};

///
void PhotonHashmap_setPhoton(struct PhotonHashmap * map, struct JensenPhoton * photon, int index) {
    JensenPhoton_setData(photon, map->photon_data, index);
}

///
void PhotonHashmap_computeGridFirstPhoton(struct PhotonHashmap * map, int index) {
    
    if (index >= map->numPhotons) {
        return;
    }
    
    if (index == 0 && map->gridIndices[index] != -1) {
        map->gridFirstPhotonIndices[map->gridIndices[index]] = index;
    }
    else {
        int currGrid = map->gridIndices[index];
        int prevGrid = map->gridIndices[index-1];

        if (currGrid != -1 && currGrid != prevGrid) {
            map->gridFirstPhotonIndices[currGrid] = index;
        }
    }
}

///
void PhotonHashmap_gatherPhotonIndices(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    // output
    __global float * photon_indices,
    int * photonsFound
    );
void PhotonHashmap_gatherPhotonIndices_v2(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    // output
    __global float * photon_indices,
    int * photonsFound
    );

int findMaxDistancePhotonIndex(
    struct PhotonHashmap * map,
    float3 intersection,
    __global float * photon_indices,
    int numPhotons
    );
void PhotonHashmap_gatherClosestPhotonsForGridIndex(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    ///
    int i, int j, int k,
    
    // output
    __global float * photon_indices,
    int * photonsFound,
    float * maxRadiusSqd);

///
void PhotonHashmap_gatherClosestPhotonsForGridIndex(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    ///
    int i, int j, int k,
    
    // output
    __global float * photon_indices,
    int * photonsFound,
    float * maxRadiusSqd
) {

    int gridIndex = PhotonHashmap_photonHashIJK(map, i, j, k);
    // find the index of the first photon in the cell
    if (map->gridFirstPhotonIndices[gridIndex] > 0) {
        int pi = map->gridFirstPhotonIndices[gridIndex];
        while (pi < map->numPhotons && map->gridIndices[pi] == gridIndex) {
            struct JensenPhoton p = JensenPhoton_fromData(map->photon_data, pi);
            // Check if the photon is on the same geometry as the intersection
            // We only store K photons. If there are less than K photons stored in the array, add the current photon to the array
            float distSqd = dot(p.position - intersection, p.position - intersection);
            if (*photonsFound < maxNumPhotonsToGather) {
                if (distSqd < maxPhotonDistance * maxPhotonDistance) {
                    photon_indices[*photonsFound] = pi;
                    *photonsFound += 1;
                    
                    *maxRadiusSqd = max(distSqd, *maxRadiusSqd);
                }
            }
            // If the array is full, find the photon with the largest distance to the intersection. If current photon's distance
            // to the intersection is smaller, replace the photon with the largest distance with the current photon
            else {
                int maxDistIndex = findMaxDistancePhotonIndex(map, intersection, photon_indices, *photonsFound);
                struct JensenPhoton farthestPhoton = JensenPhoton_fromData(map->photon_data, photon_indices[maxDistIndex]);
                
                float searchResult_squareDistance = dot(farthestPhoton.position - intersection, farthestPhoton.position - intersection);
                
                if (distSqd < searchResult_squareDistance) {
                    photon_indices[maxDistIndex] = pi;
                    if (distSqd > *maxRadiusSqd
                     || fabs(*maxRadiusSqd - searchResult_squareDistance) < 0.0001f) {
                        *maxRadiusSqd = distSqd;
                    }
                }
            }
            pi++;
        }
    }

}

bool pointInsideCube(
    float3 point,
    
    float3 cube_start,
    float3 cube_end);
bool sphereInsideCube(
    float3 sphere_position,
    float sphere_radius_sqd,
    
    float3 cube_center,
    float cube_halfwidth_sqd);
float3 PhotonHashmap_getCellBoxStart(
    struct PhotonHashmap * map,
    /// which cell
    int i, int j, int k);

///
bool pointInsideCube(
    float3 point,
    
    float3 cube_start,
    float3 cube_end) {

    return cube_start.x <= point.x && point.x <= cube_end.x
     && cube_start.y <= point.y && point.y <= cube_end.y
     && cube_start.z <= point.z && point.z <= cube_end.z;
}

///
bool sphereInsideCube(
    float3 sphere_position,
    float sphere_radius_sqd,
    
    float3 cube_center,
    float cube_halfwidth_sqd) {

    float3 cube_start = cube_center + (float3) {-cube_halfwidth_sqd, -cube_halfwidth_sqd, -cube_halfwidth_sqd};
    float3 cube_end = cube_center + (float3) {cube_halfwidth_sqd, cube_halfwidth_sqd, cube_halfwidth_sqd};

    return pointInsideCube(sphere_position + (float3) {sphere_radius_sqd, sphere_radius_sqd, sphere_radius_sqd}, cube_start, cube_end)
     && pointInsideCube(sphere_position + (float3) {-sphere_radius_sqd, -sphere_radius_sqd, -sphere_radius_sqd}, cube_start, cube_end);
}

float3 PhotonHashmap_getCellBoxStart(
    struct PhotonHashmap * map,
    /// which cell
    int i, int j, int k) {
    
    float3 start;
    
    start.x = map->xmin + (map->cellsize * (float) i);
    start.y = map->ymin + (map->cellsize * (float) j);
    start.z = map->zmin + (map->cellsize * (float) k);

    return start;
}

///
void PhotonHashmap_gatherPhotonIndices(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    // output
    __global float * photon_indices,
    int * photonsFound
    ) {
    
    int3 gridIndex = PhotonHashmap_cellIndex(map, intersection);
    int px = gridIndex.x, py = gridIndex.y, pz = gridIndex.z;
    *photonsFound = 0;
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < map->xdim
     && py >= 0 && py < map->ydim
     && pz >= 0 && pz < map->zdim) {
        
        float maxRadiusSqd = 0.0f;
        
        /// Find initial set of photons
        PhotonHashmap_gatherClosestPhotonsForGridIndex(map, maxNumPhotonsToGather, maxPhotonDistance, intersection, px, py, pz, photon_indices, photonsFound, &maxRadiusSqd);
        
        int outerBoxWidthSize = 1;
        float outerBoxWidth = map->cellsize * (float) outerBoxWidthSize;
        int largestDim = max(max(map->xdim, map->ydim), map->zdim);
        
        float3 searchBoxCenter = PhotonHashmap_getCellBoxStart(map, px, py, pz)
         + 0.5f * (float3) { map->cellsize, map->cellsize, map->cellsize };
        
        bool photonSphereInsideCube = sphereInsideCube(intersection, sqrt(maxRadiusSqd), searchBoxCenter, outerBoxWidth / 3.0f);
        bool doneSearching = *photonsFound == maxNumPhotonsToGather && photonSphereInsideCube;
        bool searchSpaceTooLarge = outerBoxWidthSize > largestDim || outerBoxWidth / 2.0f > maxPhotonDistance; // || outerBoxWidthSize > (2 * map->spacing + 1);
        
        while (!doneSearching && !searchSpaceTooLarge) {
            
            outerBoxWidthSize += 2;
            outerBoxWidth = map->cellsize * (float) outerBoxWidthSize;
            
            for (int counter = 0; counter < outerBoxWidthSize * outerBoxWidthSize * outerBoxWidthSize; counter++) {
                int boxX = (counter % outerBoxWidthSize);
                int boxY = ((counter / outerBoxWidthSize) % outerBoxWidthSize);
                int boxZ = (counter / (outerBoxWidthSize * outerBoxWidthSize));
            
                if (boxX == 1 && boxY > 0 && (boxY < outerBoxWidthSize - 1) && boxZ > 0 && (boxZ < outerBoxWidthSize - 1)) {
                    counter += outerBoxWidthSize - 2;
                }
                
                int bi = (counter % outerBoxWidthSize) - (outerBoxWidthSize / 2);
                int bj = ((counter / outerBoxWidthSize) % outerBoxWidthSize) - (outerBoxWidthSize / 2);
                int bk = (counter / (outerBoxWidthSize * outerBoxWidthSize)) - (outerBoxWidthSize / 2);
                
                int i = min(max(0, px + bi), map->xdim - 1);
                int j = min(max(0, py + bj), map->ydim - 1);
                int k = min(max(0, pz + bk), map->zdim - 1);
                
                if (i == (px + bi) && j == (py + bj) && k == (pz + bk)) {
                    PhotonHashmap_gatherClosestPhotonsForGridIndex(map, maxNumPhotonsToGather, maxPhotonDistance, intersection, i, j, k, photon_indices, photonsFound, &maxRadiusSqd);
                }
            }
            
            photonSphereInsideCube = sphereInsideCube(intersection, sqrt(maxRadiusSqd), searchBoxCenter, outerBoxWidth / 3.0f);
            doneSearching = *photonsFound == maxNumPhotonsToGather && photonSphereInsideCube;
            searchSpaceTooLarge = outerBoxWidthSize > largestDim || outerBoxWidth / 2.0f > maxPhotonDistance; // || outerBoxWidthSize > (2 * map->spacing + 1);
        }
    }
}

///
void PhotonHashmap_gatherPhotonIndices_v2(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    // output
    __global float * photon_indices,
    int * photonsFound
    ) {
    
    int3 gridIndex = PhotonHashmap_cellIndex(map, intersection);
    int px = gridIndex.x, py = gridIndex.y, pz = gridIndex.z;
    *photonsFound = 0;
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < map->xdim
     && py >= 0 && py < map->ydim
     && pz >= 0 && pz < map->zdim) {
        
        float maxRadiusSqd = 0.0f;
    
        for (int i = max(0, px - map->spacing); i < min(map->xdim, px+map->spacing+1); ++i) {
            for (int j = max(0, py - map->spacing); j < min(map->ydim, py+map->spacing+1); ++j) {
                for (int k = max(0, pz - map->spacing); k < min(map->zdim, pz+map->spacing+1); ++k) {
                    
                    PhotonHashmap_gatherClosestPhotonsForGridIndex(map, maxNumPhotonsToGather, maxPhotonDistance, intersection, i, j, k, photon_indices, photonsFound, &maxRadiusSqd);
                }
            }
        }
    }
}

///
int findMaxDistancePhotonIndex(
    struct PhotonHashmap * map,
    float3 intersection,
    __global float * photon_indices,
    int numPhotons
    ) {
    
    int index = -1;
    float squareDistance = -1.0f;
    
    for (int i = 0; i < numPhotons; ++i) {
        struct JensenPhoton photon = JensenPhoton_fromData(map->photon_data, photon_indices[i]);
        float photon_distance_i = dot(photon.position - intersection, photon.position - intersection);
        if (photon_distance_i > squareDistance) {
            index = i;
            squareDistance = photon_distance_i;
        }
    }
    
    return index;
}


#endif /* photon_hashmap_h */
