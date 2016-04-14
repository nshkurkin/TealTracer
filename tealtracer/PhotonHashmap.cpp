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
int
PhotonHashmap::findMaxDistancePhotonIndex(const std::vector<PhotonMap::PhotonIndexInfo> & photonIndices, const Eigen::Vector3f & intersection) {
    MaxDistanceSearchResult result;

    int index = -1;
    double squareDistance = -std::numeric_limits<float>::infinity();
    
    for (int i = 0; i < (int) photonIndices.size(); ++i) {
        if (photonIndices[i].squareDistance > squareDistance) {
            index = i;
            squareDistance = photonIndices[i].squareDistance;
        }
    }
    
    return index;
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

#include "TSLogger.hpp"

///
void PhotonHashmap::computeGridFirstPhotons() {
    gridFirstPhotonIndices.clear();
    gridFirstPhotonIndices.resize(xdim * ydim * zdim, -1);

    bool placedFirstGridItem = false;

    //
    for (int index = 0; index < photons.size(); index++) {
        
        // First one always has to be stored
        if (!placedFirstGridItem && gridIndices[index] != -1) {
            TSLoggerLog(std::cout, "girdIndices[", index, "]=", gridIndices[index], " gridFirstPhotonIndices.size()=", gridFirstPhotonIndices.size());
            assert(gridIndices[index] < gridFirstPhotonIndices.size());
            gridFirstPhotonIndices[gridIndices[index]] = index;
            placedFirstGridItem = true;
        }
        else {
            int currGrid = gridIndices[index];
            int prevGrid = gridIndices[index-1];

            if (currGrid != prevGrid && currGrid != -1) {
                assert(currGrid < gridFirstPhotonIndices.size());
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

    auto gridIndex = getCellIndex(intersection);
    int px = gridIndex.x(), py = gridIndex.y(), pz = gridIndex.z();
    // Find photons in neighboring cells
    std::vector<PhotonIndexInfo> neighborPhotons;
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < xdim
     && py >= 0 && py < ydim
     && pz >= 0 && pz < zdim
     && gridFirstPhotonIndices.size() > 0) {
        
        float maxRadiusSqd = -1.0f;
        
        for (int i = std::max(0, px - spacing); i < std::min(xdim, px+spacing+1); ++i) {
            for (int j = std::max(0, py - spacing); j < std::min(ydim, py+spacing+1); ++j) {
                for (int k = std::max(0, pz - spacing); k < std::min(zdim, pz+spacing+1); ++k) {
                    
                    int gridIndex = photonHash(i, j, k);
                    // find the index of the first photon in the cell
                    assert(gridIndex <= gridFirstPhotonIndices.size());
                    if (gridFirstPhotonIndices[gridIndex] != -1) {
                        int pi = gridFirstPhotonIndices[gridIndex];
                        while (pi < photons.size() && gridIndices[pi] == gridIndex) {
                            const auto & p = photons[pi];
                            // Check if the photon is on the same geometry as the intersection
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
                                auto index = findMaxDistancePhotonIndex(neighborPhotons, intersection);
                                auto searchResult = neighborPhotons[index];
                                if (distSqd < searchResult.squareDistance) {
                                    neighborPhotons[index].index = pi;
                                    neighborPhotons[index].squareDistance = distSqd;
                                    if (distSqd > maxRadiusSqd
                                     || fabs(maxRadiusSqd - searchResult.squareDistance) < epsilon) {
                                        maxRadiusSqd = distSqd;
                                    }
                                }
                            }
                            pi++;
                        }
                    }
                }
            }
        }
    }
    return neighborPhotons;
//    return accumColor;
}
