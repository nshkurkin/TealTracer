//
//  RaytracingConfig.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef RaytracingConfig_hpp
#define RaytracingConfig_hpp

#include <Eigen/Dense>

#include "PovrayScene.hpp"
#include "json.hpp"

struct RaytracingConfig {
    
    int renderOutputWidth;
    int renderOutputHeight;

    enum SupportedBRDF {
        BlinnPhong = 0, // https://en.wikipedia.org/wiki/Blinn–Phong_shading_model
        OrenNayar = 1 // https://en.wikipedia.org/wiki/Oren–Nayar_reflectance_model
    };
    
    enum ComputationDevice {
        CPU = 0,
        GPU = 1
    };
    
    ComputationDevice computationDevice;
    SupportedBRDF brdfType;
    
    int numberOfPhotonsToGather;
    float maxPhotonGatherDistance;
    int raysPerLight;
    int lumensPerLight;
    
    float photonBounceProbability;
    float photonBounceEnergyMultipler;
    
    bool usePhotonMappingForDirectIllumination;
    
    bool directIlluminationEnabled;
    bool indirectIlluminationEnabled;
    bool shadowsEnabled;
    
    Eigen::Vector3f hashmapGridStart, hashmapGridEnd;
    float hashmapSpacing, hashmapCellsize;
    
    Eigen::Vector3f Up;
    Eigen::Vector3f Forward;
    Eigen::Vector3f Right;
    
    std::shared_ptr<PovrayScene> scene;
    
    RaytracingConfig();
    void loadFromJSON(const nlohmann::json & config);
};

#endif /* RaytracingConfig_hpp */
