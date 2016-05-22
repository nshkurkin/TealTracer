//
//  PhotonHashmap.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/11/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PhotonHashmap_hpp
#define PhotonHashmap_hpp

#include <vector>
#include <Eigen/Dense>

#include "PhotonMap.hpp"
#include "JensenPhoton.hpp"

///
class PhotonHashmap : public PhotonMap {
public:
    int spacing;
    float xmin, ymin, zmin;
    float xmax, ymax, zmax;
    int xdim, ydim, zdim;
    float cellsize;
    
    const float epsilon;
    
    ///
    PhotonHashmap();
    ///
    virtual ~PhotonHashmap() {}
    
    ///
    void setDimensions(const Eigen::Vector3f & minExtent, const Eigen::Vector3f & maxExtent) {
        xmin = minExtent.x();
        ymin = minExtent.y();
        zmin = minExtent.z();
        
        xmax = maxExtent.x();
        ymax = maxExtent.y();
        zmax = maxExtent.z();
    
        // Compute photon grid dimensions
        xdim = std::ceil((xmax - xmin)/cellsize);
        ydim = std::ceil((ymax - ymin)/cellsize);
        zdim = std::ceil((zmax - zmin)/cellsize);
    }
    
    ///
    Eigen::Vector3i getCellIndex(const Eigen::Vector3f & position) const;
    ///
    int photonHash(int i, int j, int k) const;
    ///
    int photonHash(const Eigen::Vector3i & index) const;
    ///
    int getCellIndexHash(const Eigen::Vector3f & position) const;
    ///
    Eigen::Vector3i getCellIndexForHash(int hash) const;
    
    ///
    typedef PhotonIndexInfo MaxDistanceSearchResult;
    
    // Find the index of the photon with the largest distance to the intersection
    int findMaxDistancePhotonIndex(const std::vector<PhotonMap::PhotonIndexInfo> & photonIndices);
    
    /// Call this after filling "photons" with the relevant content.
    virtual void buildMap();
    /// Call this after building the spatial hash.
    ///
    /// NOTE: flux = totalEnergy/(float)numPhotons;
    virtual std::vector<PhotonIndexInfo> gatherPhotonsIndices(
        int maxNumPhotonsToGather,
        float maxPhotonDistance,
        const Eigen::Vector3f & intersection);
    ///
    virtual std::vector<PhotonMap::PhotonIndexInfo> gatherPhotonsIndices_v2(
        int maxNumPhotonsToGather,
        float maxPhotonDistance,
        const Eigen::Vector3f & intersection);
    
private:
    ///
    void mapPhotonsToGrid();
    ///
    void computeGridFirstPhotons();
    ///
    void gatherClosestPhotonsForGridIndex(
        int maxNumPhotonsToGather,
        float maxPhotonDistance,
        const Eigen::Vector3f & intersection,
        ///
        int i, int j, int k,
        
        // output
        std::vector<PhotonIndexInfo> & neighborPhotons,
        float * maxRadiusSqd
    );
    
    Eigen::Vector3f getCellBoxStart(
        /// which cell
        int i, int j, int k);
    Eigen::Vector3f getCellBoxEnd(
        /// which cell
        int i, int j, int k);
    
public:
    std::vector<int> gridFirstPhotonIndices;
    std::vector<int> gridIndices;
    
};

#endif /* PhotonHashmap_hpp */
