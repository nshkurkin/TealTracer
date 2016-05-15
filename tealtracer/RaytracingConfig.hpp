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
#include <vector>
#include <string>

#include "PovrayScene.hpp"
#include "json.hpp"

class RaytracingConfig {
public:
    bool enabled;
    std::string title;
    bool controlsCamera;
    
    int renderOutputWidth;
    int renderOutputHeight;

    enum SupportedBRDF {
        BlinnPhong = 0, // https://en.wikipedia.org/wiki/Blinn–Phong_shading_model
        OrenNayar = 1 // https://en.wikipedia.org/wiki/Oren–Nayar_reflectance_model
    };
    
    enum SupportedPhotonMap {
        KDTree = 0,
        HashGrid = 1,
        TileFrustum = 2
    };
    
    enum ComputationDevice {
        CPU = 0,
        GPU = 1
    };
    
    ComputationDevice computationDevice;
    SupportedBRDF brdfType;
    SupportedPhotonMap supportedPhotonMap;
    
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
    
    ///
    static Eigen::Vector3f vec3FromData(const std::vector<double> & data) {
        return Eigen::Vector3f(data[0], data[1], data[2]);
    }
};

#endif /* RaytracingConfig_hpp */
