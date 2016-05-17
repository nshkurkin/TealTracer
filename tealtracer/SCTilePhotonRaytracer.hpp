//
//  SCTilePhotonRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SCTilePhotonRaytracer_hpp
#define SCTilePhotonRaytracer_hpp

#include "SCPhotonMapper.hpp"
#include "PhotonTiler.hpp"

class SCTilePhotonRaytracer : public SingleCoreRaytracer {
public:

    ///
    SCTilePhotonRaytracer();
    
    /// step(x) is a geometrically distributed random number generator
    ///     representing samples of the number of trials until the first success
    ///     a Bernoulli random variable with success probability 1/x.
    int step(double x);
    
    ///
    virtual void start();
    /// called in "start" to setup parameters from ".config"
    virtual void configure();
    
    ///
    virtual void raytraceScene();
    
protected:

    std::shared_ptr<PhotonTiler> photonTiler;

};

#endif /* SCTilePhotonRaytracer_hpp */
