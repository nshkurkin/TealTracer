//
//  SCKDTreeRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SCKDTreeRaytracer.hpp"

///
SCKDTreeRaytracer::SCKDTreeRaytracer() : SCPhotonMapper() {

}

///
void
SCKDTreeRaytracer::configure() {

    config.supportedPhotonMap = RaytracingConfig::KDTree;
    SCPhotonMapper::configure();
}
