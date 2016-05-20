//
//  scene_objects.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef scene_objects_h
#define scene_objects_h

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

__constant const unsigned int kPovrayLightSourceStride = 7;

///
struct PovrayLightSourceData PovrayLightSourceData_fromData(__global float * data);

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
enum ObjectType {
    SphereObjectType = 0,
    PlaneObjectType = 1,
    NumObjectTypes = 2
};

///
struct PovraySphereData {
    float3 position;
    float radius;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
    
    float id;
};

__constant unsigned int kPovraySphereStride = 3 + 1 + kPovrayPigmentStride + kPovrayFinishStride + 1;

///
struct PovraySphereData PovraySphereData_fromData(__global float * data);

///
struct PovrayPlaneData {
    float3 normal;
    float distance;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
    
    float id;
};

__constant unsigned int kPovrayPlaneStride = 3 + 1 + kPovrayPigmentStride + kPovrayFinishStride + 1;

///
struct PovrayPlaneData PovrayPlaneData_fromData(__global float * data);

////////////////////////////////////////////////////////////////////////////

///
struct PovrayLightSourceData PovrayLightSourceData_fromData(__global float * data) {
    struct PovrayLightSourceData result;
    
    result.position = (float3) { data[0], data[1], data[2] };
    result.color = (float4) { data[3], data[4], data[5], data[6] };
    
    return result;
}

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
    result.id = data[12];
    
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
    result.id = data[12];
    
    return result;
}

#endif /* scene_objects_h */
