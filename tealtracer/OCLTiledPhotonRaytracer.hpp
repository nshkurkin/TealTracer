//
//  OCLTiledPhotonRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OCLTiledPhotonRaytracer_hpp
#define OCLTiledPhotonRaytracer_hpp

#include "OpenCLRaytracer.hpp"
#include "CLPovrayElementData.hpp"
#include "PhotonTiler.hpp"

#include <memory>

class OCLTiledPhotonRaytracer : public OpenCLRaytracer {
public:

    OCLTiledPhotonRaytracer();
    
    ///
    virtual void start();
    
    packed_struct PackedPlane {
        cl_float plane_normal_x, plane_normal_y, plane_normal_z;
        cl_float plane_distance;
        void fromPlane(const PhotonTiler::Plane & plane);
    };
    
    packed_struct PackedFrustum {
        struct PackedPlane planes[4];
        void fromFrustum(const PhotonTiler::Frustum & frustum);
    };
    
    packed_struct PackedTile {
        struct PackedFrustum frustum;
        void fromTile(const PhotonTiler::Tile & tile);
    };
    
    virtual void ocl_raytraceSetup();

    ///
    void ocl_emitPhotons();
    
    virtual void ocl_buildAndFillTiles();
    virtual void ocl_raytraceRays();
    
private:

    std::shared_ptr<PhotonTiler> photonTiler;

};

#endif /* OCLTiledPhotonRaytracer_hpp */
