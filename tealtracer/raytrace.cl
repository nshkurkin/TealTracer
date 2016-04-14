//
//
//
//
//

__constant float3 Up = (float3){0.0, 1.0, 0.0};
__constant float3 Forward = (float3){0.0, 0.0, -1.0};
__constant float3 Right = (float3){1.0, 0.0, 0.0};

///
struct PovrayCameraData {
    float3 location;
    float3 up;
    float3 right;
    float3 lookAt;
};

///
struct PovrayLightSourceData {
    float3 position;
    float4 color;
};

///
struct PovrayPigment {
    float4 color;
};

__constant const unsigned int kPovrayPigmentStride = 4;

///
struct PovrayFinish {
    float ambient;
    float diffuse;
    float specular;
    float roughness;
};

__constant const unsigned int kPovrayFinishStride = 4;

///
struct PovraySphereData {
    float3 position;
    float radius;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
};

__constant unsigned int kPovraySphereStride = 3 + 1 + kPovrayPigmentStride + kPovrayFinishStride;

///
struct PovraySphereData PovraySphereData_fromData(__global float * data);

///
struct PovrayPlaneData {
    float3 normal;
    float distance;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
};

__constant unsigned int kPovrayPlaneStride = 3 + 1 + kPovrayPigmentStride + kPovrayFinishStride;

///
struct PovrayPlaneData PovrayPlaneData_fromData(__global float * data);

////////////////////////////////////////////////////////////////////////////

///
struct PovraySphereData PovraySphereData_fromData(__global float * data) {
    struct PovraySphereData result;
    
    result.position = (float3) { data[0], data[1], data[2] };
    result.radius = data[3];
    result.pigment.color = (float4) { data[4], data[5], data[6], data[7] };
    result.finish.ambient = data[8];
    result.finish.diffuse = data[9];
    result.finish.specular = data[10];
    result.finish.roughness= data[11];
    
    return result;
}

///
struct PovrayPlaneData PovrayPlaneData_fromData(__global float * data) {
    struct PovrayPlaneData result;
    
    result.normal = (float3) { data[0], data[1], data[2] };
    result.distance = data[3];
    result.pigment.color = (float4) { data[4], data[5], data[6], data[7] };
    result.finish.ambient = data[8];
    result.finish.diffuse = data[9];
    result.finish.specular = data[10];
    result.finish.roughness= data[11];
    
    return result;
}


////////////////////////////////////////////////////////////////////////////

///
struct mat4x4 {
    float4 col[4];
};

///
struct ubyte4 {
    unsigned char bytes[4];
};

///
struct ubyte4 ubyte4_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

///
void mat4x4_set(struct mat4x4 * mat, int row, int col, float val);
///
float mat4x4_get(struct mat4x4 * mat, int row, int col);
///
float4 mat4x4_getRow(struct mat4x4 * mat, int row);

///
float4 mat4x4_mult4x1(struct mat4x4 * mat, float4 vec);
///
void mat4x4_loadIdentity(struct mat4x4 * mat);
///
void mat4x4_loadLookAt(struct mat4x4 * result, float3 eye, float3 center, float3 up);

////////////////////////////////////////////////////////////////////////////

///
struct ubyte4 ubyte4_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    struct ubyte4 result;
    result.bytes[0] = r;
    result.bytes[1] = g;
    result.bytes[2] = b;
    result.bytes[3] = a;
    return result;
}

///
void mat4x4_set(struct mat4x4 * mat, int row, int col, float val) {
    ((float *) &(mat->col[col]))[row] = val;
}

///
float mat4x4_get(struct mat4x4 * mat, int row, int col) {
    float4 column = mat->col[col];
    return ((float *) &column)[row];
}

///
float4 mat4x4_getRow(struct mat4x4 * mat, int row) {
    float4 result;
    
    result.x = mat4x4_get(mat, row, 0);
    result.y = mat4x4_get(mat, row, 1);
    result.z = mat4x4_get(mat, row, 2);
    result.w = mat4x4_get(mat, row, 3);
    
    return result;
}

///
float4 mat4x4_mult4x1(struct mat4x4 * mat, float4 vec) {
    float4 result;
    
    result.x = dot(vec, mat4x4_getRow(mat, 0));
    result.y = dot(vec, mat4x4_getRow(mat, 1));
    result.z = dot(vec, mat4x4_getRow(mat, 2));
    result.w = dot(vec, mat4x4_getRow(mat, 3));
    
    return result;
}

///
void mat4x4_loadIdentity(struct mat4x4 * mat) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                mat4x4_set(mat,i,j, 1.0);
            }
            else {
                mat4x4_set(mat,i,j, 0.0);
            }
        }
    }
}

///
void mat4x4_loadLookAt(struct mat4x4 * result, float3 eye, float3 center, float3 up) {
    float3 f = normalize(center - eye);
    float3 s = normalize(cross(f, up));
    float3 u = cross(s, f);
    
    mat4x4_loadIdentity(result);
    
    mat4x4_set(result,0,0, s.x);
    mat4x4_set(result,1,0, s.y);
    mat4x4_set(result,2,0, s.z);
    mat4x4_set(result,0,1, u.x);
    mat4x4_set(result,1,1, u.y);
    mat4x4_set(result,2,1, u.z);
    mat4x4_set(result,0,2, -f.x);
    mat4x4_set(result,1,2, -f.y);
    mat4x4_set(result,2,2, -f.z);
    
    mat4x4_set(result,3,0, -dot(s, eye));
    mat4x4_set(result,3,1, -dot(u, eye));
    mat4x4_set(result,3,2, dot(f, eye));
}

////////////////////////////////////////////////////////////////////////////

///
enum ObjectType {
    SphereObjectType = 0,
    PlaneObjectType = 1,
    NumObjectTypes = 2
};

///
struct RayIntersectionResult {
    bool intersected;
    float timeOfIntersection;
    
    enum ObjectType type;
    __global float * dataPtr;
};

///
struct RayIntersectionResult closest_intersection(
    __global float * data,
    unsigned int numDataElements,
    unsigned int dataStride,
    float3 rayOrigin, float3 rayDirection,
    enum ObjectType type);

///
void sphere_intersect(__global float * dataPtr, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result);
///
void plane_intersect(__global float * dataPtr, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result);


///
///
struct RayIntersectionResult closest_intersection(
    __global float * data,
    unsigned int numDataElements,
    unsigned int dataStride,
    float3 rayOrigin, float3 rayDirection,
    enum ObjectType type) {

    struct RayIntersectionResult result;
    result.intersected = false;
    result.timeOfIntersection = INFINITY;
    
    for (unsigned int i = 0; i < numDataElements; i++) {
        struct RayIntersectionResult currentResult;
        currentResult.intersected = false;
        currentResult.timeOfIntersection = INFINITY;
    
        __global float * dataStart = &data[i*dataStride];
        currentResult.dataPtr = dataStart;
        
        switch (type) {
            case SphereObjectType:
                sphere_intersect(dataStart, rayOrigin, rayDirection, &currentResult);
                break;
            case PlaneObjectType:
                plane_intersect(dataStart, rayOrigin, rayDirection, &currentResult);
                break;
            default:
                break;
        }
        
        if (currentResult.intersected
         && currentResult.timeOfIntersection < result.timeOfIntersection) {
            result = currentResult;
        }
    }
    
    result.type = type;
    return result;
}


///
void sphere_intersect(__global float * dataPtr, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result) {
    
    struct PovraySphereData data = PovraySphereData_fromData(dataPtr);
    
    float A = dot(rayDirection, rayDirection);
    float B = 2.0 * dot(rayOrigin - data.position, rayDirection);
    float C = dot(rayOrigin - data.position, rayOrigin - data.position) - data.radius * data.radius;
    
    float radical = B*B - 4.0*A*C;
    if (radical >= 0) {
        float sqrRadical = sqrt(radical);
        float t0 = (-B + sqrRadical)/(2.0 * A);
        float t1 = (-B - sqrRadical)/(2.0 * A);
        result->intersected = t0 >= 0 || t1 >= 0;
        if (t0 >= 0 && t1 >= 0) {
            result->timeOfIntersection = min(t0, t1);
        }
        else if (t0 >= 0) {
            result->timeOfIntersection = t0;
        }
        else if (t1 >= 0) {
            result->timeOfIntersection = t1;
        }
    }
}

///
void plane_intersect(__global float * dataPtr, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result) {
    
    struct PovrayPlaneData data = PovrayPlaneData_fromData(dataPtr);
    
    float product = dot(rayDirection, data.normal);
    if (product > 0.0001 || product < -0.0001) {
        result->timeOfIntersection = -(dot(rayOrigin, data.normal) - data.distance) / product;
    }
    
    result->intersected = result->timeOfIntersection >= 0.0;
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

// Called over "photons.size()" photons
global kernel void photonmap_mapPhotonsToGrid(
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
    
    if (index >= map.numPhotons) {
        return;
    }
    
    struct JensenPhoton photon = JensenPhoton_fromData(map.photon_floats, map.photon_objTypes, map.photon_geomptrs, index);
    
    int3 cellIndex = PhotonHashmap_cellIndex(&map, photon.position);
    if (cellIndex.x >= 0 && cellIndex.x < map.xdim
     && cellIndex.y >= 0 && cellIndex.y < map.ydim
     && cellIndex.z >= 0 && cellIndex.z < map.zdim) {
    
        map.gridIndices[index] = PhotonHashmap_photonHash3i(&map, cellIndex);
    }
}

///////////////////////////////////////////////////////////////////////////
///
/// KERNEL: photonmap_computeGridFirstPhotons
///
/// SYNOPSIS: Called after "mapPhotonsToGrid" has been run on the hashmap data.
///
/// NOTE: Called over "photons.size()" photons
///

global kernel void photonmap_computeGridFirstPhotons(
    // Grid specification
    PHOTON_HASHMAP_META_PARAMS) {

    struct PhotonHashmap map;
    PHOTON_HASHMAP_SET_META_PARAMS();
    
    int index = (int) get_global_id(0);
    
    if (index == 0 && map.gridIndices[index] != -1) {
        map.gridFirstPhotonIndices[map.gridIndices[index]] = index;
    }
    else {
        int currGrid = map.gridIndices[index];
        int prevGrid = map.gridIndices[index-1];

        if (currGrid != -1 && currGrid != prevGrid) {
            map.gridFirstPhotonIndices[currGrid] = index;
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
    );
int findMaxDistancePhotonIndex(
    __global float * photon_distances,
    int numPhotons
    );

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
                                if (distSqd < photon_distances[maxDistIndex]) {
                                    photon_indices[maxDistIndex] = pi;
                                    photon_distances[maxDistIndex] = distSqd;
                                    if (distSqd > maxRadiusSqd
                                     || fabs(maxRadiusSqd - photon_distances[maxDistIndex]) < 0.0001) {
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
        switch (bestIntersection.type) {
            case SphereObjectType: {
                struct PovraySphereData data = PovraySphereData_fromData(bestIntersection.dataPtr);
                color = ubyte4_make(data.pigment.color.x * 255, data.pigment.color.y * 255, data.pigment.color.z * 255, data.pigment.color.w * 255);
                break;
            }
            case PlaneObjectType: {
                struct PovrayPlaneData data = PovrayPlaneData_fromData(bestIntersection.dataPtr);
                color = ubyte4_make(data.pigment.color.x * 255, data.pigment.color.y * 255, data.pigment.color.z * 255, data.pigment.color.w * 255);
            }
                break;
            default:
                break;
        }
    }
    
    /// Update the output
    for (int i = 0; i < 4; i++) {
        imageOutput[4 * (px + imageWidth * py) + i] = color.bytes[i];
    }
}

