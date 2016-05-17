//
//  PhotonTiler.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/12/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PhotonTiler_hpp
#define PhotonTiler_hpp

#include <vector>
#include <cassert>
#include <memory>

#include "PovraySceneElements.hpp"
#include "JensenPhoton.hpp"
#include "Ray.hpp"

/// From "Toward Practical Real-Time Photon Mapping: Efficient GPU Density Estimation" 6
///
//  A tiled algorithm inserts copies of each photons in buckets corresponding to the screen-space tiles it might affect. This allowed a second pass to shade all pixels within a tile from a com- mon subset of photons that fit within shared memory for a compute shader. This yields a significant DRAM bandwidth savings com- pared to the other methods we’ve described, which load each pixel or photon multiple times. In the case where there are more photons in a tile than fit in memory, we make multiple passes over a tile, processing a sizable chunks of the photons in the tile in each pass.
//  During the process of categorizing the photons into tiles, a tiled al- gorithm can cull photons that do not intersect the depth range of the scene within a tile. In contrast, each 3D or 2.5D photon volume used in rasterization-based methods only affects pixels within a nar- row depth range, yet the single-sided hardware depth test requires that those algrithims launch shading threads for either all pixels in front of or all pixels behind a certain depth. In scenes with high depth variance, that may launch many shading threads that immedi- ately terminate, reducing vector lane occupancy without providing useful computation.
//  Listing 5 outlines the tile insertion pass. Our actual implementation further partitions tiles into chunks each containing a bounded pho- ton count; this is a tradeoff of simplicity for flexibility (e.g., new photons can be directly appended to partial chunks). Each photon consumes little memory, so we duplicate and embed them directly in the chunks instead of chasing pointers. This also uses the cache efficiently and enables coalesced parallel memory fetches. Subfig- ure 3f shows the bounding frusta for tiles in a simple scene; all photons accessed by pixels in a tile intersect its frustum.

/// Tile Insertion
///
//let m = number of tiles
//let photonCount[m], nextPhotonIndex[m] = arrays of 0s
//
//for each photon P : # counting pass
//    for each tile T with frustum intersecting the effect sphere of P :
//        photonCount[T] += 1
//
//for each tile T : # allocation pass
//    allocate photonCount[T] photons in tilePhotons[T]
//
//for each photon P : # copy pass
//    for each tile T with frustum intersecting the effect sphere of P :
//        s = atomicGetAndIncrement(nextPhotonIndex[T])
//        tilePhotons[T][s] = P

/// Tile Gathering
///
//for each n×n tile T: # Thread launch
//
//    let photon[ ] = shared memory array
//    for each pixel (x, y) in T :
//        load 1/n2 of tile contents into photon[ ]
//
//    for each pixel (x, y) in T with visible surface X:
//        # Iterate through tile contents
//        i = step(k2) − 1 # See section 2.3.2
//        count = 0; sum = 0
//        while i < photon.length:
//            ++count
//            P = photon[i]
//            sum += contribution of P at X
//            i += step(k2)
//        image[x, y] += sum ∗ photon.length / count

///
class PhotonTiler {
public:

    /// NOTE: Photons are positioned in world space
    std::vector<JensenPhoton> photons;

    ///
    PhotonTiler();
    
    ///
    struct Plane {
        Eigen::Vector3f normal;
        float distance;
        
        ///
        Plane();
        ///
        Plane(const Eigen::Vector3f & normal, float distance);
        
        ///
        float distanceToPoint(const Eigen::Vector3f & point);
    };
    
    ///
    struct Frustum {
        std::vector<Plane> planes;
        
        ///
        Frustum();
        
        ///
        bool intersectsOrContainsSphere(const Eigen::Vector3f & sphereCenter, float sphereRadius);
        ///
        void setPlanesFromRayCast(const Eigen::Vector3f & tileProjectionOrigin, const Eigen::Vector3f & tileCenter, const Eigen::Vector3f & camera_up, const Eigen::Vector3f & camera_right, float boxWidth, float boxHeight);
    };
    
    ///
    struct Tile {
        struct Frustum frustum;
        
        Tile();
    };
    
    std::vector<struct Tile> tiles;
    std::vector<std::vector<JensenPhoton>> tilePhotons;
    
    ///
    virtual void generateTiles(
        const int imageWidth, const int imageHeight,
        const int tileWidth, const int tileHeight,
        const Eigen::Vector3f & cameraPosition,
        const FrenetFrame & viewFrame);
    
    ///
    int tileIndexForPixel(
        const int imageWidth, const int imageHeight,
        const int tileWidth, const int tileHeight,
        const int px, const int py) const;
    
    /// Before "buildMap" is called, the tiles must be configured with the current camera
    virtual void buildMap(float photonEffectRadius);
    
    

};

#endif /* PhotonTiler_hpp */
