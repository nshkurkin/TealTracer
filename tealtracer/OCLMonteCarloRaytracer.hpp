//
//  OCLMonteCarloRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OCLMonteCarloRaytracer_hpp
#define OCLMonteCarloRaytracer_hpp

#include "OpenCLRaytracer.hpp"

class OCLMonteCarloRaytracer : public OpenCLRaytracer {
public:

    virtual void ocl_raytraceSetup();
    virtual void ocl_raytraceRays();

};

#endif /* OCLMonteCarloRaytracer_hpp */
