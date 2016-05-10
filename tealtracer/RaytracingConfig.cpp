//
//  RaytracingConfig.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "RaytracingConfig.hpp"

    
RaytracingConfig::RaytracingConfig() {
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
    renderOutputWidth = config["outputWidth"].get<int>();
    renderOutputHeight = config["outputHeight"].get<int>();
    
    computationDevice = (ComputationDevice) config["computationDevice"].get<int>();
    brdfType = (SupportedBRDF) config["brdfType"].get<int>();
    
    numberOfPhotonsToGather = config["numberOfPhotonsToGather"].get<int>();
    raysPerLight = config["raysPerLight"].get<int>();
    float maxPhotonGatherDistance = config["maxPhotonGatherDistance"].get<double>();
    if (maxPhotonGatherDistance != -1.0f) {
        this->maxPhotonGatherDistance = maxPhotonGatherDistance;
    }
    else {
        this->maxPhotonGatherDistance = std::numeric_limits<float>::infinity();
    }
    lumensPerLight = config["lumensPerLight"].get<int>();
    
    photonBounceProbability = config["photonBounceProbability"].get<double>();
    photonBounceEnergyMultipler = config["photonBounceEnergyMultipler"].get<double>();
    
    usePhotonMappingForDirectIllumination = config["usePhotonMappingForDirectIllumination"].get<bool>();
    
    directIlluminationEnabled = config["directIlluminationEnabled"].get<bool>();
    indirectIlluminationEnabled = config["indirectIlluminationEnabled"].get<bool>();
    shadowsEnabled = config["shadowsEnabled"].get<bool>();
    
    auto vec3FromJSON = [](const std::vector<double> & data) {
        return Eigen::Vector3f(data[0], data[1], data[2]);
    };
    
    hashmapCellsize = config["Hashmap_properties"]["cellsize"].get<double>();
    hashmapSpacing = config["Hashmap_properties"]["spacing"].get<int>();
    hashmapGridStart = vec3FromJSON(config["Hashmap_properties"]["gridStart"].get<std::vector<double>>());
    hashmapGridEnd << vec3FromJSON(config["Hashmap_properties"]["gridEnd"].get<std::vector<double>>());
    
    Up = vec3FromJSON(config["Up"].get<std::vector<double>>());
    Forward = vec3FromJSON(config["Forward"].get<std::vector<double>>());
    Right = vec3FromJSON(config["Right"].get<std::vector<double>>());
    
    scene = PovrayScene::loadScene(config["povrayScene"].get<std::string>());
}
