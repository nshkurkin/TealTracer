//
//  OCLOptimizedTiledPhotonRaytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/19/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OCLOptimizedTiledPhotonRaytracer_hpp
#define OCLOptimizedTiledPhotonRaytracer_hpp

#include "OpenCLRaytracer.hpp"
#include "CLPovrayElementData.hpp"
#include "PhotonTiler.hpp"

#include <memory>

class OCLOptimizedTiledPhotonRaytracer : public OpenCLRaytracer {
public:

    OCLOptimizedTiledPhotonRaytracer();
    
    ///
    virtual void start();
    ///
    virtual void enqueueRaytrace();
    
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

    ///
    std::string tilePhotonBufferName(int whichTile) const {
        return std::string("tilePhotons_") + std::to_string(whichTile);
    }
    
    unsigned int photonEmissionSeed;

    std::vector<cl_int> tilePhotonCount;
    CLPovrayCameraData cachedCameraData;
    std::shared_ptr<PhotonTiler> photonTiler;
};

#endif /* OCLOptimizedTiledPhotonRaytracer_hpp */
