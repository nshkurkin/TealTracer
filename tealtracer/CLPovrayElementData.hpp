//
//  CLPovrayElementData.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/9/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef CLPovrayElementData_hpp
#define CLPovrayElementData_hpp

#include "compute_engine.hpp"
#include "PovraySceneElements.hpp"

///
struct CLPovrayPigment {
    float4 color;
    
    CLPovrayPigment() : color(0,0,0,1) {}
    CLPovrayPigment(const PovrayPigment & data) : color(data.color.x(), data.color.y(), data.color.z(), data.color.w()) {}
};

///
struct CLPovrayFinish {
    cl_float ambient;
    cl_float diffuse;
    
    CLPovrayFinish() : ambient(0.0), diffuse(0.0) {}
    CLPovrayFinish(const PovrayFinish & data) : ambient(data.ambient), diffuse(data.diffuse) {}
};

///
struct CLPovrayCameraData {
    float3 location;
    float3 up;
    float3 right;
    float3 lookAt;
    
    CLPovrayCameraData() : location(0,0,0), up(0,1,0), right(1,0,0), lookAt(0,0,-1) {}
    CLPovrayCameraData(const PovrayCameraData & data) : location(data.location.x(), data.location.y(), data.location.z()), up(data.up.x(), data.up.y(), data.up.z()), right(data.right.x(), data.right.y(), data.right.z()), lookAt(data.lookAt.x(), data.lookAt.y(), data.lookAt.z()) {}
};

///
struct CLPovrayLightSourceData {
    float3 position;
    float4 color;
    
    CLPovrayLightSourceData() : position(0,0,0), color(0,0,0,1) {}
    CLPovrayLightSourceData(const PovrayLightSourceData & data) : position(data.position.x(), data.position.y(), data.position.z()), color(data.color.x(), data.color.y(), data.color.z(), data.color.w()) {}
};

///
struct CLPovraySphereData {
    float3 position;
    cl_float radius;
    
    CLPovrayPigment pigment;
    CLPovrayFinish finish;
    
    CLPovraySphereData() : position(0,0,0), radius(1.0), pigment(), finish() {}
    CLPovraySphereData(const PovraySphereData & data) : position(data.position.x(), data.position.y(), data.position.z()), radius(data.radius), pigment(data.pigment), finish(data.finish) {}
};

///
///
struct CLPovrayPlaneData {
    float3 normal;
    cl_float distance;
    
    CLPovrayPigment pigment;
    CLPovrayFinish finish;
    
    CLPovrayPlaneData() : normal(0,1,0), distance(0), pigment(), finish() {}
    CLPovrayPlaneData(const PovrayPlaneData & data) : normal(data.normal.x(), data.normal.y(), data.normal.z()), distance(data.distance), pigment(data.pigment), finish(data.finish) {}
};

#endif /* CLPovrayElementData_hpp */








