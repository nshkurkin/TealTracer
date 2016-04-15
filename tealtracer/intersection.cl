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
        
        if (result->timeOfIntersection > 0) {
            result->rayOrigin = rayOrigin;
            result->rayDirection = rayDirection;
            result->surfaceNormal = normalize(RayIntersectionResult_locationOfIntersection(result) - data.position);
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
    
    result->rayOrigin = rayOrigin;
    result->rayDirection = rayDirection;
    result->surfaceNormal = data.normal;
    
    result->intersected = result->timeOfIntersection >= 0.0;
}

#endif /* intersection_h */
