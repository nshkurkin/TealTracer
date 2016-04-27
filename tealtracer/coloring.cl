//
//  coloring.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef coloring_h
#define coloring_h

#include "scene_objects.cl"
#include "intersection.cl"
#include "photon_hashmap.cl"

enum BRDFType {
    BlinnPhong = 0,
    OrenNayar = 1
};

///
RGBf computeOutputEnergyForHit(
    enum BRDFType brdf,
    struct RayIntersectionResult hitResult,
    float3 toLight, float3 toViewer);
///
RGBf computeOutputEnergyForBRDF(
    enum BRDFType brdf,
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal);
///
RGBf computeBlinnPhongOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal);
RGBf computeOrenNayarOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal);
///
RGBf computeOutputEnergyForHitWithPhotonMap(
    enum BRDFType brdf,
    struct RayIntersectionResult hitResult,
    struct PhotonHashmap * map,
    int maxNumPhotonsToGather,
    float maxGatherDistance,
    float3 toViewer,
    __global int * photon_indices,
    __global float * photon_distances);

///
RGBf computeBlinnPhongOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal) {
    
    RGBf output;
        
    float multiplier = 0.0f;
    
    multiplier += finish.ambient;
    multiplier += finish.diffuse * max(0.0f, dot(surfaceNormal, toLight));
    
    if (finish.roughness > 0.001f) {
        float3 halfwayVector = normalize(toLight + toViewer);
        multiplier += finish.specular * pow(max(0.0f, dot(surfaceNormal, halfwayVector)), 1.0f / finish.roughness);
    }
    
    output = pigment.color.xyz * multiplier;
    
    output.x *= source.x;
    output.y *= source.y;
    output.z *= source.z;
    
    return output;
}

///
RGBf computeOrenNayarOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal) {
    
    RGBf output;
        
    /// http://ruh.li/GraphicsOrenNayar.html
    float angleVN = acos(dot(surfaceNormal, toViewer));
    float angleLN = acos(dot(surfaceNormal, toLight));
    
    float alpha = max(angleVN, angleLN);
    float beta = min(angleVN, angleLN);
    
    float gamma = dot(toViewer - surfaceNormal * dot(toViewer, surfaceNormal), toLight - surfaceNormal*dot(toLight,surfaceNormal));

    float roughnessSquared = finish.roughness * finish.roughness;

    // calculate A and B
    float A = 1.0f - 0.5f * (roughnessSquared / (roughnessSquared + 0.57f));
    float B = 0.45f * (roughnessSquared / (roughnessSquared + 0.09f));
    float C = sin(alpha) * tan(beta);
    
    // put it all together
    float multiplier = max(0.0f, dot(surfaceNormal, toLight)) * (A + B * max(0.0f, gamma) * C);
    
    ///
    output = pigment.color.xyz * multiplier;
    
    output.x *= source.x;
    output.y *= source.y;
    output.z *= source.z;
    
    return output;
}

///
RGBf computeOutputEnergyForBRDF(
    enum BRDFType brdf,
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal) {

    RGBf output = (float3) {0.0f, 0.0f, 0.0f};
    switch (brdf) {
        case BlinnPhong : {
            output = computeBlinnPhongOutputEnergy(pigment, finish, source, toLight, toViewer, surfaceNormal);
            break;
        }
        case OrenNayar : {
            output = computeOrenNayarOutputEnergy(pigment, finish, source, toLight, toViewer, surfaceNormal);
            break;
        }
        default:
            break;
    }
    return output;
}

///
RGBf computeOutputEnergyForHit(
    enum BRDFType brdf,
    struct RayIntersectionResult hitResult,
    float3 toLight, float3 toViewer) {
    
    RGBf output = (float3) {0.0f, 0.0f, 0.0f};
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
    
    switch (hitResult.type) {
        case SphereObjectType: {
            struct PovraySphereData data = PovraySphereData_fromData(hitResult.dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        }
        case PlaneObjectType: {
            struct PovrayPlaneData data = PovrayPlaneData_fromData(hitResult.dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        };
        default: {
            break;
        }
    }
    
    output = computeOutputEnergyForBRDF(brdf, pigment, finish, (RGBf) {1.0f,1.0f,1.0f}, toLight, toViewer, hitResult.surfaceNormal);
    
    return output;
}

RGBf computeOutputEnergyForHitWithPhotonMap(
    enum BRDFType brdf,
    struct RayIntersectionResult hitResult,
    struct PhotonHashmap * map,
    int maxNumPhotonsToGather,
    float maxGatherDistance,
    float3 toViewer,
    __global int * photon_indices,
    __global float * photon_distances
) {
    
    RGBf output = (RGBf) {0,0,0};
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
    
    switch (hitResult.type) {
        case SphereObjectType: {
            struct PovraySphereData data = PovraySphereData_fromData(hitResult.dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        }
        case PlaneObjectType: {
            struct PovrayPlaneData data = PovrayPlaneData_fromData(hitResult.dataPtr);
            pigment = data.pigment;
            finish = data.finish;
            break;
        };
        default: {
            break;
        }
    }
    
    int numPhotonsFound;
    PhotonHashmap_gatherPhotonIndices(map, maxNumPhotonsToGather, maxGatherDistance, RayIntersectionResult_locationOfIntersection(&hitResult), photon_indices, photon_distances, &numPhotonsFound);
    
    float maxSqrDist = 0.001f;
    
    //  Accumulate radiance of the K nearest photons
    for (int i = 0; i < numPhotonsFound; ++i) {
        
        struct JensenPhoton p = PhotonHashmap_getPhoton(map, photon_indices[i]);
        RGBf photonEnergy = (RGBf) {0,0,0};
        
        if (photon_distances[i] > maxSqrDist) {
            maxSqrDist = photon_distances[i];
        }
        
        photonEnergy = computeOutputEnergyForBRDF(brdf, pigment, finish, p.energy, -p.incomingDirection, toViewer, hitResult.surfaceNormal);
        
        output += photonEnergy;
    }
    
    if (numPhotonsFound > 0) {
        output = output / (float) (M_PI * maxSqrDist);
    }
    
    return output;
}


#endif /* coloring_h */
