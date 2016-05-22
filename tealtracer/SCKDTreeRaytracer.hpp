//
//  SCKDTreeRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef SCKDTreeRaytracer_hpp
#define SCKDTreeRaytracer_hpp

#include "SCPhotonMapper.hpp"

class SCKDTreeRaytracer : public SCPhotonMapper {
public:

    SCKDTreeRaytracer();
    
    ///
    virtual void configure();
};

#endif /* SCKDTreeRaytracer_hpp */
