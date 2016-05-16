//
//  PhotonEmitter.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/16/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PhotonEmitter_hpp
#define PhotonEmitter_hpp

#include "SingleCoreRaytracer.hpp"
#include "TSRandomValueGenerator.hpp"
#include "JensenPhoton.hpp"
#include "Ray.hpp"

struct PhotonEmitter {

    ///
    void emitPhotons(
        SingleCoreRaytracer * raytracer,
        std::vector<JensenPhoton> & photons);
    
private:
    
    ///
    void processEmittedPhoton(
        SingleCoreRaytracer * raytracer,
        std::vector<JensenPhoton> & photons,
        
        ///
        RGBf sourceLightEnergy,
        const Ray & initialRay,
        
        ///
        bool * photonStored);

};


#endif /* PhotonEmitter_hpp */
