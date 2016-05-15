//
//  SCPhotonMapper.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SCPhotonMapper_hpp
#define SCPhotonMapper_hpp

#include "SingleCoreRaytracer.hpp"
#include "PhotonMap.hpp"

class SCPhotonMapper : public SingleCoreRaytracer {
public:

    SCPhotonMapper() {
    
    }
    
    ///
    virtual void start();
    ///
    virtual void configure();

    ///
    virtual void raytraceScene();
    
    ///
    virtual void buildPhotonMap();
    
    ///
    virtual RGBf computeOutputEnergyForHitUsingPhotonMap(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy);
    
protected:
    
    ///
    void emitPhotons();
    
    ///
    std::shared_ptr<PhotonMap> photonMap;
    
private:
    
    ///
    void processEmittedPhoton(RGBf sourceLightEnergy, const Ray & initialRay, bool * photonStored);
};

#endif /* SCPhotonMapper_hpp */
