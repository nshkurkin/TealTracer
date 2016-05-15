//
//  RaytracingConfig.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "RaytracingConfig.hpp"
#include "stl_extensions.hpp"
    
RaytracingConfig::RaytracingConfig() {

    enabled = false;
    title = "";
    controlsCamera = false;

    renderOutputWidth = 0;
    renderOutputHeight = 0;

    computationDevice = ComputationDevice::CPU;
    brdfType = SupportedBRDF::BlinnPhong;

    numberOfPhotonsToGather = 0;
    maxPhotonGatherDistance = 0.0f;
    raysPerLight = 0;
    lumensPerLight = 0;

    photonBounceProbability = 0.0f;
    photonBounceEnergyMultipler = 0.0f;

    usePhotonMappingForDirectIllumination = false;

    directIlluminationEnabled = false;
    indirectIlluminationEnabled = false;
    shadowsEnabled = false;

    hashmapGridStart = Eigen::Vector3f::Zero();
    hashmapGridEnd = Eigen::Vector3f::Zero();
    hashmapSpacing = 0.0f;
    hashmapCellsize = 0.0f;

    Up = Eigen::Vector3f::Zero();
    Forward = Eigen::Vector3f::Zero();
    Right = Eigen::Vector3f::Zero();

    scene = nullptr;
}

void RaytracingConfig::loadFromJSON(const nlohmann::json & config) {

    enabled = config.get<bool>("enabled");
    title = config.get<std::string>("title");
    controlsCamera = config.get<bool>("controlsCamera");

    renderOutputWidth = config.get<int>("outputWidth");
    renderOutputHeight = config.get<int>("outputHeight");
    
    computationDevice = (ComputationDevice) config.get<int>("computationDevice");
    brdfType = (SupportedBRDF) config.get<int>("brdfType");
    supportedPhotonMap = (SupportedPhotonMap) config.get<int>("supportedPhotonMap");
    
    numberOfPhotonsToGather = config.get<int>("numberOfPhotonsToGather");
    raysPerLight = config.get<int>("raysPerLight");
    float maxPhotonGatherDistance = config.get<double>("maxPhotonGatherDistance", -1.0);
    if (maxPhotonGatherDistance != -1.0f) {
        this->maxPhotonGatherDistance = maxPhotonGatherDistance;
    }
    else {
        this->maxPhotonGatherDistance = std::numeric_limits<float>::infinity();
    }
    lumensPerLight = config.get<int>("lumensPerLight");
    
    photonBounceProbability = config.get<double>("photonBounceProbability");
    photonBounceEnergyMultipler = config.get<double>("photonBounceEnergyMultipler");
    
    usePhotonMappingForDirectIllumination = config.get<bool>("usePhotonMappingForDirectIllumination");
    
    directIlluminationEnabled = config.get<bool>("directIlluminationEnabled");
    indirectIlluminationEnabled = config.get<bool>("indirectIlluminationEnabled");
    shadowsEnabled = config.get<bool>("shadowsEnabled");
    
    if (config.has("Hashmap_properties")) {
        hashmapCellsize = config["Hashmap_properties"].get<double>("cellsize");
        hashmapSpacing = config["Hashmap_properties"].get<int>("spacing");
        hashmapGridStart = vec3FromData(config["Hashmap_properties"].get<std::vector<double>>("gridStart", make_vector<double>(0,0,0)));
        hashmapGridEnd = vec3FromData(config["Hashmap_properties"].get<std::vector<double>>("gridEnd", make_vector<double>(0,0,0)));
    }
}
