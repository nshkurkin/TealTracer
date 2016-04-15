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
RGBf computeBlinnPhongOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal);
RGBf computeOrenNayarOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal);

///
RGBf computeBlinnPhongOutputEnergy(
    struct PovrayPigment pigment, struct PovrayFinish finish,
    RGBf source, float3 toLight, float3 toViewer, float3 surfaceNormal) {
    
    RGBf output;
        
    float multiplier = 0.0;
    
    multiplier += finish.ambient;
    multiplier += finish.diffuse * max(0.0f, dot(surfaceNormal, toLight));
    
    if (finish.roughness > 0.001) {
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
    float A = 1.0 - 0.5 * (roughnessSquared / (roughnessSquared + 0.57));
    float B = 0.45 * (roughnessSquared / (roughnessSquared + 0.09));
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

    RGBf output = (float3) {0, 0, 0};
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
RGBf computeOutputEnergyHit(
    enum BRDFType brdf,
    struct RayIntersectionResult hitResult,
    float3 toLight, float3 toViewer) {
    
    RGBf output = (float3) {0, 0, 0};
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
    
    output = computeOutputEnergyForBRDF(brdf, pigment, finish, (RGBf) {1,1,1}, toLight, toViewer, hitResult.surfaceNormal);
    
    return output;
}


//RGBf computeOutputEnergyForHitWithPhotonMap(struct RayIntersectionResult hitResult, struct PhotonHashmap * map, float3 toViewer) {
//    
//    RGBf output = RGBf::Zero();
//    brdf->pigment = *hitResult.element->pigment();
//    brdf->finish = *hitResult.element->finish();
//    
//    if (usePhotonMap) {
//        auto photonInfo = photonMap->gatherPhotonsIndices(numberOfPhotonsToGather, std::numeric_limits<float>::infinity(), hitResult.hit.locationOfIntersection());
//        
//        float maxSqrDist = 0.001;
//        //  Accumulate radiance of the K nearest photons
//        for (int i = 0; i < photonInfo.size(); ++i) {
//            
//            const auto & p = photonMap->photons[photonInfo[i].index];
//            
//            RGBf photonEnergy = RGBf::Zero(), surfaceEnergy = RGBf::Zero();
//            if (photonInfo[i].squareDistance > maxSqrDist) {
//                maxSqrDist = photonInfo[i].squareDistance;
//            }
//            
//            photonEnergy = brdf->computeColor(rgbe2rgb(p.energy), -p.incomingDirection.vector(), toViewer, hitResult.hit.surfaceNormal);
//            surfaceEnergy = brdf->computeColor(RGBf(1,1,1), -p.incomingDirection.vector(), toViewer, hitResult.hit.surfaceNormal);
//            
//            output.x() += photonEnergy.x() * surfaceEnergy.x();
//            output.y() += photonEnergy.y() * surfaceEnergy.y();
//            output.z() += photonEnergy.z() * surfaceEnergy.z();
//        }
//        
//        output = output / (M_PI * maxSqrDist);
//        
//    }
//    else {
//       output = brdf->computeColor(RGBf(1,1,1), toLight, toViewer, hitResult.hit.surfaceNormal);
//    }
//    
//    return output;
//}


#endif /* coloring_h */
