//
//  SCHashGridRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SCHashGridRaytracer.hpp"

///
SCHashGridRaytracer::SCHashGridRaytracer() : SCPhotonMapper() {

}

///
void
SCHashGridRaytracer::configure() {

    config.supportedPhotonMap = RaytracingConfig::HashGrid;
    SCPhotonMapper::configure();
}
