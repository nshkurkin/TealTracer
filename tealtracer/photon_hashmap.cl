//
//  photon_hashmap.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef photon_hashmap_h
#define photon_hashmap_h

////////////////////////////////////////////////////////////////////////////

///
struct ubyte4 {
    unsigned char bytes[4];
};

///
struct ubyte4 ubyte4_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

///
struct ubyte4 ubyte4_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    struct ubyte4 result;
    result.bytes[0] = r;
    result.bytes[1] = g;
    result.bytes[2] = b;
    result.bytes[3] = a;
    return result;
}

////////////////////////////////////////////////////////////////////////////

///
/// TODO: Work area. Figure out how photons fit into a large sorting algorithm
///         on the GPU. i.e. we can either sort the actual structs or we might
///         consider sorting photon indices instead, but then do we have data
///         locality problems with our sort?
///

typedef float3 RGBf;

///
struct JensenPhoton {

    ///
    float3 position;
    float3 incomingDirection;
    ///
    RGBf energy;
    
    ///
    enum ObjectType geometryType;
    __global float * geometryDataPtr;
};

struct JensenPhoton JensenPhoton_fromData(
    __global float * photon_floats,
    __global int * photon_ints,
    __global float * __global * photon_geomptrs,
    int whichPhoton);
void JensenPhoton_setData(
    struct JensenPhoton * photon,
    
    __global float * photon_floats,
    __global int * photon_ints,
    __global float * __global * photon_geomptrs,
    int whichPhoton);

///
struct JensenPhoton JensenPhoton_fromData(
    __global float * photon_floats,
    __global int * photon_ints,
    __global float * __global * photon_geomptrs,
    int whichPhoton) {
 
    struct JensenPhoton photon;
    
    __global float * photon_floats_start = &(photon_floats[whichPhoton * 9]);
    
    photon.position = (float3) { photon_floats_start[0], photon_floats_start[1], photon_floats_start[2] };
    photon.incomingDirection = (float3) { photon_floats_start[3], photon_floats_start[4], photon_floats_start[5] };
    photon.energy = (float3) { photon_floats_start[6], photon_floats_start[7], photon_floats_start[8] };
    
    photon.geometryType = (enum ObjectType) photon_ints[whichPhoton];
    photon.geometryDataPtr = photon_geomptrs[whichPhoton];
    
    return photon;
}

///
void JensenPhoton_setData(
    struct JensenPhoton * photon,
    
    __global float * photon_floats,
    __global int * photon_ints,
    __global float * __global * photon_geomptrs,
    int whichPhoton) {

    __global float * photon_floats_start = &(photon_floats[whichPhoton * 9]);
    
    photon_floats_start[0] = photon->position.x;
    photon_floats_start[1] = photon->position.y;
    photon_floats_start[2] = photon->position.z;
    
    photon_floats_start[3 + 0] = photon->incomingDirection.x;
    photon_floats_start[3 + 1] = photon->incomingDirection.y;
    photon_floats_start[3 + 2] = photon->incomingDirection.z;
    
    photon_floats_start[6 + 0] = photon->energy.x;
    photon_floats_start[6 + 1] = photon->energy.y;
    photon_floats_start[6 + 2] = photon->energy.z;
    
    photon_ints[whichPhoton] = (int) photon->geometryType;
    photon_geomptrs[whichPhoton] = photon->geometryDataPtr;
}
    
//////////////////////////////////////////////////////////////////////////////
///
///
struct PhotonHashmap {
    int spacing;
    float xmin, ymin, zmin;
    float xmax, ymax, zmax;
    int xdim, ydim, zdim;
    float cellsize;
    int numPhotons;
    
    /// Photon heap data
    __global float * photon_floats;
    __global int * photon_objTypes;
    __global float * __global * photon_geomptrs;
    
    /// Photon metadata
    __global int * gridIndices; // size: numPhotons
    __global int * gridFirstPhotonIndices; // size: xdim * ydim * zdim
};

int PhotonHashmap_photonHashIJK(struct PhotonHashmap * map, int i, int j, int k);
int PhotonHashmap_photonHash3i(struct PhotonHashmap * map, int3 index);
int PhotonHashmap_cellIndexHash(struct PhotonHashmap * map, float3 position);
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
void PhotonHashmap_mapPhotonToGrid(struct PhotonHashmap * map, int index);
void PhotonHashmap_computeGridFirstPhoton(struct PhotonHashmap * map, int index);
struct JensenPhoton PhotonHashmap_getPhoton(struct PhotonHashmap * map, int index);
void PhotonHashmap_setPhoton(struct PhotonHashmap * map, struct JensenPhoton * photon, int index);

///
struct JensenPhoton PhotonHashmap_getPhoton(struct PhotonHashmap * map, int index) {
    return JensenPhoton_fromData(
        map->photon_floats,
        map->photon_objTypes,
        map->photon_geomptrs,
        index);
};

///
void PhotonHashmap_setPhoton(struct PhotonHashmap * map, struct JensenPhoton * photon, int index) {
    JensenPhoton_setData(
        photon,
        map->photon_floats,
        map->photon_objTypes,
        map->photon_geomptrs,
        index);
}

///
void PhotonHashmap_gatherPhotonIndices(
    struct PhotonHashmap * map,
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const float3 intersection,
    // output
    __global int * photon_indices,
    __global float * photon_distances,
    int * photonsFound
    );
int findMaxDistancePhotonIndex(
    __global float * photon_distances,
    int numPhotons
    );

// Called over "photons.size()" photons
void PhotonHashmap_mapPhotonToGrid(struct PhotonHashmap * map, int index) {
    
    if (index >= map->numPhotons) {
        return;
    }
    
    struct JensenPhoton photon = JensenPhoton_fromData(map->photon_floats, map->photon_objTypes, map->photon_geomptrs, index);
    
    int3 cellIndex = PhotonHashmap_cellIndex(map, photon.position);
    if (cellIndex.x >= 0 && cellIndex.x < map->xdim
     && cellIndex.y >= 0 && cellIndex.y < map->ydim
     && cellIndex.z >= 0 && cellIndex.z < map->zdim) {
    
        map->gridIndices[index] = PhotonHashmap_photonHash3i(map, cellIndex);
    }
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
    __global int * photon_indices,
    __global float * photon_distances,
    int * photonsFound
    ) {
    
    int3 gridIndex = PhotonHashmap_cellIndex(map, intersection);
    int px = gridIndex.x, py = gridIndex.y, pz = gridIndex.z;
    *photonsFound = 0;
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < map->xdim
     && py >= 0 && py < map->ydim
     && pz >= 0 && pz < map->zdim) {
        
        float maxRadiusSqd = -1.0f;
        
        for (int i = max(0, px - map->spacing); i < min(map->xdim, px+map->spacing+1); ++i) {
            for (int j = max(0, py - map->spacing); j < min(map->ydim, py+map->spacing+1); ++j) {
                for (int k = max(0, pz - map->spacing); k < min(map->zdim, pz+map->spacing+1); ++k) {
                    
                    int gridIndex = PhotonHashmap_photonHashIJK(map, i, j, k);
                    // find the index of the first photon in the cell
                    if (map->gridFirstPhotonIndices[gridIndex] != -1) {
                        int pi = map->gridFirstPhotonIndices[gridIndex];
                        while (pi < map->numPhotons && map->gridIndices[pi] == gridIndex) {
                            struct JensenPhoton p = JensenPhoton_fromData(map->photon_floats, map->photon_objTypes, map->photon_geomptrs, pi);
                            // Check if the photon is on the same geometry as the intersection
                            // We only store K photons. If there are less than K photons stored in the array, add the current photon to the array
                            float distSqd = dot(p.position - intersection, p.position - intersection);
                            if (*photonsFound < maxNumPhotonsToGather) {
                                if (distSqd < maxPhotonDistance * maxPhotonDistance) {
                                    photon_indices[*photonsFound] = pi;
                                    photon_distances[*photonsFound] = distSqd;
                                    *photonsFound += 1;
                                    
                                    maxRadiusSqd = max(distSqd, maxRadiusSqd);
                                }
                            }
                            // If the array is full, find the photon with the largest distance to the intersection. If current photon's distance
                            // to the intersection is smaller, replace the photon with the largest distance with the current photon
                            else {
                                int maxDistIndex = findMaxDistancePhotonIndex(photon_distances, *photonsFound);
                                
                                int searchResult_index = photon_indices[maxDistIndex];
                                float searchResult_squareDistance = photon_distances[maxDistIndex];
                                
                                if (distSqd < searchResult_squareDistance) {
                                    photon_indices[searchResult_index] = pi;
                                    photon_distances[searchResult_index] = distSqd;
                                    if (distSqd > maxRadiusSqd
                                     || fabs(maxRadiusSqd - photon_distances[searchResult_index]) < 0.0001) {
                                        maxRadiusSqd = distSqd;
                                    }
                                }
                            }
                            pi++;
                        }
                    }
                }
            }
        }
    }
}

///
int findMaxDistancePhotonIndex(
    __global float * photon_distances,
    int numPhotons
    ) {
    
    int index = -1;
    float squareDistance = -1.0;
    
    for (int i = 0; i < numPhotons; ++i) {
        if (photon_distances[i] > squareDistance) {
            index = i;
            squareDistance = photon_distances[i];
        }
    }
    
    return index;
}


#endif /* photon_hashmap_h */
