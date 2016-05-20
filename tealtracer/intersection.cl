//
//  intersection.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef intersection_h
#define intersection_h

#include "scene_objects.cl"

////////////////////////////////////////////////////////////////////////////

///
struct RayIntersectionResult {
    bool intersected;
    float timeOfIntersection;
    
    float3 rayOrigin;
    float3 rayDirection;
    float3 surfaceNormal;
    
    enum ObjectType type;
    __global float * dataPtr;
    float geomId;
};

///
float3 RayIntersectionResult_locationOfIntersection(struct RayIntersectionResult * result);
float3 RayIntersectionResult_outgoingDirection(struct RayIntersectionResult * result);

///
float3 RayIntersectionResult_locationOfIntersection(struct RayIntersectionResult * result) {
    return result->rayOrigin + (result->rayDirection * result->timeOfIntersection);
}

///
float3 RayIntersectionResult_outgoingDirection(struct RayIntersectionResult * result) {
    /// http://www.cosinekitty.com/raytrace/chapter10_reflection.html
    return result->rayDirection - 2.0f * dot(result->rayDirection, result->surfaceNormal) * result->surfaceNormal;
}

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
void next_intersection(
    ///
    int whichDataElement,
    __global float * data,
    unsigned int numDataElements,
    unsigned int dataStride,
    float3 rayOrigin, float3 rayDirection,
    enum ObjectType type,
    /// Output
    struct RayIntersectionResult * result);

///
void next_intersection(
    ///
    int whichDataElement,
    __global float * data,
    unsigned int numDataElements,
    unsigned int dataStride,
    float3 rayOrigin, float3 rayDirection,
    enum ObjectType type,
    /// Output
    struct RayIntersectionResult * result) {
    
    result->intersected = false;
    result->timeOfIntersection = INFINITY;
    
    __global float * dataStart = &data[whichDataElement*dataStride];
    result->dataPtr = dataStart;
    
    switch (type) {
        case SphereObjectType:
            sphere_intersect(dataStart, rayOrigin, rayDirection, result);
            break;
        case PlaneObjectType:
            plane_intersect(dataStart, rayOrigin, rayDirection, result);
            break;
        default:
            break;
    }
    
    result->type = type;
}

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
        
        next_intersection(i, data, numDataElements, dataStride, rayOrigin, rayDirection, type, &currentResult);
        
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
    float B = 2.0f * dot(rayOrigin - data.position, rayDirection);
    float C = dot(rayOrigin - data.position, rayOrigin - data.position) - data.radius * data.radius;
    
    float radical = B*B - 4.0f*A*C;
    if (radical >= 0) {
        float sqrRadical = sqrt(radical);
        float t0 = (-B + sqrRadical)/(2.0f * A);
        float t1 = (-B - sqrRadical)/(2.0f * A);
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
        
        if (result->timeOfIntersection > 0) {
            result->rayOrigin = rayOrigin;
            result->rayDirection = rayDirection;
            result->surfaceNormal = normalize(RayIntersectionResult_locationOfIntersection(result) - data.position);
            result->geomId = data.id;
        }
    }
}

///
void plane_intersect(__global float * dataPtr, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result) {
    
    struct PovrayPlaneData data = PovrayPlaneData_fromData(dataPtr);
    
    float product = dot(rayDirection, data.normal);
    if (product > 0.001f || product < -0.001f) {
        result->timeOfIntersection = -(dot(rayOrigin, data.normal) - data.distance) / product;
    }
    
    result->rayOrigin = rayOrigin;
    result->rayDirection = rayDirection;
    result->surfaceNormal = data.normal;
    
    result->intersected = result->timeOfIntersection > 0.0f;
    result->geomId = data.id;
}

#endif /* intersection_h */
