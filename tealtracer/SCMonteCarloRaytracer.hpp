//
//  SCMonteCarloRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SCMonteCarloRaytracer_hpp
#define SCMonteCarloRaytracer_hpp

#include "SingleCoreRaytracer.hpp"

class SCMonteCarloRaytracer : public SingleCoreRaytracer {
public:
    SCMonteCarloRaytracer() {
    
    }
    
    virtual void raytraceScene();
};

#endif /* SCMonteCarloRaytracer_hpp */
