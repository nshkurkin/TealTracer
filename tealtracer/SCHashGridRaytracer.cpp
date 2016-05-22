//
//  SCHashGridRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SCHashGridRaytracer.hpp"
#include "PhotonHashmap.hpp"

///
SCHashGridRaytracer::SCHashGridRaytracer() : SCPhotonMapper() {

}

///
void
SCHashGridRaytracer::configure() {

    config.supportedPhotonMap = RaytracingConfig::HashGrid;
    SCPhotonMapper::configure();
}

///
RGBf
SCHashGridRaytracer::computeOutputEnergyForHitUsingPhotonMap(
    const PovrayScene::InstersectionResult & hitResult,
    const Eigen::Vector3f & toViewer,
    const RGBf & sourceEnergy)
{
    
    auto intersection = hitResult.hit.locationOfIntersection();
    std::shared_ptr<PhotonHashmap> map = std::dynamic_pointer_cast<PhotonHashmap>(photonMap);
    float maxGatherDistance = config.maxPhotonGatherDistance;
    
    auto gridIndex = map->getCellIndex(intersection);
    int px = gridIndex.x(), py = gridIndex.y(), pz = gridIndex.z();
    RGBf photonEnergy = RGBf(0,0,0);
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < map->xdim
     && py >= 0 && py < map->ydim
     && pz >= 0 && pz < map->zdim) {
        
        int photonsSampled = 0;
        float maxRadiusSqd = 0.0f;
        int halfSideLength = ceil(maxGatherDistance / map->cellsize);
        
        for (int i = std::max<int>(0, px - halfSideLength); i < std::min<int>(map->xdim, px+halfSideLength+1); ++i) {
            for (int j = std::max<int>(0, py - halfSideLength); j < std::min<int>(map->ydim, py+halfSideLength+1); ++j) {
                for (int k = std::max<int>(0, pz - halfSideLength); k < std::min<int>(map->zdim, pz+halfSideLength+1); ++k) {
                    
                    
                    int gridHash = map->photonHash(i,j,k);
                    // find the index of the first photon in the cell
                    if (map->gridFirstPhotonIndices[gridHash] > 0) {
                        int pi = map->gridFirstPhotonIndices[gridHash];
                        while (pi < map->photons.size() && map->gridIndices[pi] == gridHash) {
                            auto & p = map->photons[pi];
                            // Check if the photon is on the same geometry as the intersection and within the effect sphere
                            float distSqd = (p.position - intersection).dot(p.position - intersection);
                            if (hitResult.element->id() == p.flags.geometryIndex
                             && distSqd < maxGatherDistance * maxGatherDistance) {
                                
                                photonEnergy += computeOutputEnergyForHit(hitResult, -p.incomingDirection.vector(), toViewer, rgbe2rgb(p.energy));
                                photonsSampled++;
                                maxRadiusSqd = std::max<float>(distSqd, maxRadiusSqd);
                            }
                            
                            pi++;
                        }
                    }
                    ///
                }
            }
        }
        
        if (photonsSampled > 0) {
            photonEnergy = photonEnergy * (float) (1.0f/(M_PI * maxRadiusSqd));
        }
    }
        
    return photonEnergy;
}
