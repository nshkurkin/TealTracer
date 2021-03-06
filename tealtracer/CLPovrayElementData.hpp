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
#include "stl_extensions.hpp"

#include <vector>

///
struct CLPovrayPigment {
    float4 color;
    
    CLPovrayPigment() : color(0,0,0,1) {}
    CLPovrayPigment(const PovrayPigment & data) : color(data.color.x(), data.color.y(), data.color.z(), data.color.w()) {}
    
    void writeOutData(std::vector<cl_float> & data) {
        data.push_back(color.x);
        data.push_back(color.y);
        data.push_back(color.z);
        data.push_back(color.w);
    }
};

///
struct CLPovrayFinish {
    cl_float ambient;
    cl_float diffuse;
    cl_float specular;
    cl_float roughness;
    
    CLPovrayFinish() : ambient(0.0), diffuse(0.0), specular(0.0), roughness(0.0) {}
    CLPovrayFinish(const PovrayFinish & data) : ambient(data.ambient), diffuse(data.diffuse), specular(data.specular), roughness(data.roughness) {}
    
    void writeOutData(std::vector<cl_float> & data) {
        data.push_back(ambient);
        data.push_back(diffuse);
        data.push_back(specular);
        data.push_back(roughness);
    }
};

///
struct CLPovrayCameraData {
    float3 location;
    float3 up;
    float3 right;
    float3 lookAt;
    
    CLPovrayCameraData() : location(0,0,0), up(0,1,0), right(1,0,0), lookAt(0,0,-1) {}
    CLPovrayCameraData(const PovrayCameraData & data) : location(data.location.x(), data.location.y(), data.location.z()), up(data.up.x(), data.up.y(), data.up.z()), right(data.right.x(), data.right.y(), data.right.z()), lookAt(data.lookAt.x(), data.lookAt.y(), data.lookAt.z()) {}
    
    FrenetFrame basisVectors() const {
        return FrenetFrame(Eigen::Vector3f(lookAt.x, lookAt.y, lookAt.z), Eigen::Vector3f(up.x, up.y, up.z) , Eigen::Vector3f(right.x, right.y, right.z));
    }
    
    Eigen::Vector3f getLocation() const {
        return Eigen::Vector3f(location.x, location.y, location.z);
    }
};

///
struct CLPovrayLightSourceData {
    float3 position;
    float4 color;
    
    CLPovrayLightSourceData() : position(0,0,0), color(0,0,0,1) {}
    CLPovrayLightSourceData(const PovrayLightSourceData & data) : position(data.position.x(), data.position.y(), data.position.z()), color(data.color.x(), data.color.y(), data.color.z(), data.color.w()) {}
    
    void writeOutData(std::vector<cl_float> & data) {
        data.push_back(position.x);
        data.push_back(position.y);
        data.push_back(position.z);
        data.push_back(color.x);
        data.push_back(color.y);
        data.push_back(color.z);
        data.push_back(color.w);
    }
};

///
struct CLPovraySphereData {
    float3 position;
    cl_float radius;
    
    CLPovrayPigment pigment;
    CLPovrayFinish finish;
    
    cl_float id;
    
    CLPovraySphereData() : position(0,0,0), radius(0.0), pigment(), finish(), id(0) {}
    CLPovraySphereData(const PovraySphereData & data) : position(data.position.x(), data.position.y(), data.position.z()), radius(data.radius), pigment(data.pigment), finish(data.finish), id(data.id) {}
    
    void writeOutData(std::vector<cl_float> & data) {
        data.push_back(position.x);
        data.push_back(position.y);
        data.push_back(position.z);
        data.push_back(radius);
        pigment.writeOutData(data);
        finish.writeOutData(data);
        data.push_back(id);
    }
};

///
///
struct CLPovrayPlaneData {
    float3 normal;
    cl_float distance;
    
    CLPovrayPigment pigment;
    CLPovrayFinish finish;
    
    cl_float id;
    
    CLPovrayPlaneData() : normal(0,0,0), distance(0), pigment(), finish(), id(0) {}
    CLPovrayPlaneData(const PovrayPlaneData & data) : normal(data.normal.x(), data.normal.y(), data.normal.z()), distance(data.distance), pigment(data.pigment), finish(data.finish), id(data.id) {}
    
    void writeOutData(std::vector<cl_float> & data) {
        data.push_back(normal.x);
        data.push_back(normal.y);
        data.push_back(normal.z);
        data.push_back(distance);
        pigment.writeOutData(data);
        finish.writeOutData(data);
        data.push_back(id);
    }
};

///
packed_struct CLPackedPhoton {
    cl_float pos_x, pos_y, pos_z;
    cl_float dir_x, dir_y, dir_z;
    cl_float ene_x, ene_y, ene_z;
    
    cl_float geomId;
};

const size_t CLPackedPhoton_kNumFloats = sizeof(CLPackedPhoton{}) / sizeof(cl_float);

#endif /* CLPovrayElementData_hpp */








