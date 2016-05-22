//
//  SCHashGridRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SCHashGridRaytracer_hpp
#define SCHashGridRaytracer_hpp

#include "SCPhotonMapper.hpp"

class SCHashGridRaytracer : public SCPhotonMapper {
public:

    SCHashGridRaytracer();
    
    ///
    virtual void configure();
    
    ///
    virtual RGBf computeOutputEnergyForHitUsingPhotonMap(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy);
};

#endif /* SCHashGridRaytracer_hpp */
