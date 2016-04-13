//
//  PhotonHashmap.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/11/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PhotonHashmap.hpp"

///
PhotonHashmap::PhotonHashmap() : epsilon(0.0001)  {
    spacing = 2;
    cellsize = 2.0;
    
    setDimensions(Eigen::Vector3f(-0.5,-0.5,-0.5), Eigen::Vector3f(0.5,0.5,0.5));
}

///
Eigen::Vector3i PhotonHashmap::getCellIndex(const Eigen::Vector3f & position) const {
    Eigen::Vector3i index;
    
    index.x() = std::floor((position.x() - xmin)/cellsize);
    index.y() = std::floor((position.y() - ymin)/cellsize);
    index.z() = std::floor((position.z() - zmin)/cellsize);
    
    return index;
}

///
int PhotonHashmap::photonHash(int i, int j, int k) const {
    return i + (j * xdim) + (k * xdim * ydim);
}

///
int PhotonHashmap::photonHash(const Eigen::Vector3i & index) const {
    return photonHash(index.x(), index.y(), index.z());
}

///
int PhotonHashmap::getCellIndexHash(const Eigen::Vector3f & position) const {
    return photonHash(getCellIndex(position));
}

///
PhotonHashmap::MaxDistanceSearchResult
PhotonHashmap::findMaxDistancePhotonIndex(const std::vector<PhotonMap::PhotonIndexInfo> & photonIndices, const Eigen::Vector3f & intersection) {
    MaxDistanceSearchResult result;

    result.index = -1;
    result.squareDistance = -std::numeric_limits<float>::infinity();
    
    for (int i = 0; i < (int) photonIndices.size(); ++i) {
        if (photonIndices[i].squareDistance > result.squareDistance) {
            result = photonIndices[i];
        }
    }
    
    return result;
}

///
void PhotonHashmap::buildMap() {
    std::sort(photons.begin(), photons.end(), [&](const JensenPhoton & lhs, const JensenPhoton & rhs) {
        return getCellIndexHash(lhs.position) < getCellIndexHash(rhs.position);
    });
    
    // calculate hash value for each photon.
    mapPhotonsToGrid();
    
    // calculate starting photon index for each hashID
    computeGridFirstPhotons();
}

///
void PhotonHashmap::mapPhotonsToGrid() {
    gridIndices.clear();
    gridIndices.resize(photons.size(), -1);
    
    ///
    for (int index = 0; index < photons.size(); index++) {
        const auto & photon = photons[index];
        
        // find which gridcell this photon is in
        auto cellIndex = getCellIndex(photon.position);

        if (cellIndex.x() < 0 || cellIndex.x() >= xdim
         || cellIndex.y() < 0 || cellIndex.y() >= ydim
         || cellIndex.z() < 0 || cellIndex.z() >= zdim) {
            continue;
        }

        //use hash funcion to assign a grid index to photon
        gridIndices[index] = photonHash(cellIndex);
    }
}

///
void PhotonHashmap::computeGridFirstPhotons() {
    gridFirstPhotonIndices.clear();
    gridFirstPhotonIndices.resize(xdim * ydim * zdim, -1);

    //
    for (int index = 0; index < photons.size(); index++) {
        // First one always has to be stored
        if (index == 0) {
            gridFirstPhotonIndices[gridIndices[index]] = index;
        }
        else {
            int currGrid = gridIndices[index];
            int prevGrid = gridIndices[index-1];

            if (currGrid != prevGrid) {
                gridFirstPhotonIndices[currGrid] = index;
            }
        }
    }
}

#include "TSLogger.hpp"

///
std::vector<PhotonMap::PhotonIndexInfo>
PhotonHashmap::gatherPhotonsIndices(
    int maxNumPhotonsToGather,
    float maxPhotonDistance,
    const Eigen::Vector3f & intersection) {

//    RGBf accumColor = RGBf::Zero();
    auto gridIndex = getCellIndex(intersection);
    int px = gridIndex.x(), py = gridIndex.y(), pz = gridIndex.z();
    // Find photons in neighboring cells
    std::vector<PhotonIndexInfo> neighborPhotons;
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < xdim
     && py >= 0 && py < ydim
     && pz >= 0 && pz < zdim) {
        
        float maxRadiusSqd = -1.0f;
        
        for (int i = std::max(0, px - spacing); i < std::min(xdim, px+spacing+1); ++i) {
            for (int j = std::max(0, py - spacing); j < std::min(ydim, py+spacing+1); ++j) {
                for (int k = std::max(0, pz - spacing); k < std::min(zdim, pz+spacing+1); ++k) {
                    
                    int gridIndex = photonHash(i, j, k);
                    // find the index of the first photon in the cell
                    assert(gridIndex <= gridFirstPhotonIndices.size());
                    int firstPhotonIndex = gridFirstPhotonIndices[gridIndex];
                    if (firstPhotonIndex != -1) {
                        int pi = firstPhotonIndex;
                        while (pi < photons.size() && gridIndices[pi] == gridIndex) {
                            const auto & p = photons[pi];
                            // Check if the photon is on the same geometry as the intersection
//                            if (p.flags.geometryIndex == (uint16_t) intersectedGeomId) {
                                // We only store K photons. If there are less than K photons stored in the array, add the current photon to the array
                                float distSqd = (p.position - intersection).dot(p.position - intersection);
                                if (neighborPhotons.size() < maxNumPhotonsToGather) {
                                    if (distSqd < maxPhotonDistance * maxPhotonDistance) {
                                        neighborPhotons.push_back(PhotonIndexInfo(pi, distSqd));
                                        maxRadiusSqd = std::max<float>(distSqd, maxRadiusSqd);
                                    }
                                }
                                // If the array is full, find the photon with the largest distance to the intersection. If current photon's distance
                                // to the intersection is smaller, replace the photon with the largest distance with the current photon
                                else {
                                    auto searchResult = findMaxDistancePhotonIndex(neighborPhotons, intersection);
                                    if (distSqd < searchResult.squareDistance) {
                                        neighborPhotons[searchResult.index].index = pi;
                                        neighborPhotons[searchResult.index].squareDistance = distSqd;
                                        if (distSqd > maxRadiusSqd
                                         || fabs(maxRadiusSqd - searchResult.squareDistance) < epsilon) {
                                            maxRadiusSqd = distSqd;
                                        }
                                    }
                                }
//                            }
                            pi++;
                        }
                    }
                }
            }
        }
        
        
        
        // if there are more than 0 photons in the neighborhood, find sqr of maxDistanceSquared
//        float maxRadius = (maxRadiusSqd > 0.0) ? sqrt(maxRadiusSqd) : -1.0f;
        // Accumulate radiance of the K nearest photons
//        if (neighborPhotons.size() > 0) {
//            for (int i = 0; i < neighborPhotons.size(); ++i) {
//                const auto & p = photons[neighborPhotons[i]];
//                float diffuse = std::max(0.0f, normal.dot(-p.incomingDirection.vector()));
//                accumColor += diffuse * rgbe2rgb(p.energy);
//            }
//            
//            accumColor = accumColor * flux;// / (M_PI * maxRadiusSqd);
//        }
    }
    return neighborPhotons;
//    return accumColor;
}
