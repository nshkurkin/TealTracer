//
//  PhotonMap.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PhotonMap.hpp"

///
PhotonMap::PhotonMap() : epsilon(0.0001)  {
    spacing = 1;
    
    xmin = 0;
    ymin = 0;
    zmin = 0;
    
    xmax = 0;
    ymax = 0;
    zmax = 0;
    
    xdim = 1;
    ydim = 1;
    zdim = 1;
    
    cellsize = 1;
}

///
Eigen::Vector3i PhotonMap::getCellIndex(const Eigen::Vector3f & position) const {
    Eigen::Vector3i index;
    
    index.x() = std::floor((position.x() - xmin)/cellsize);
    index.y() = std::floor((position.y() - ymin)/cellsize);
    index.z() = std::floor((position.z() - zmin)/cellsize);
    
    return index;
}

///
int PhotonMap::photonHash(int i, int j, int k) const {
    return i + (j * xdim) + (k * xdim * ydim);
}

///
int PhotonMap::photonHash(const Eigen::Vector3i & index) const {
    return photonHash(index.x(), index.y(), index.z());
}

///
int PhotonMap::getCellIndexHash(const Eigen::Vector3f & position) const {
    return photonHash(getCellIndex(position));
}

///
PhotonMap::MaxDistanceSearchResult::MaxDistanceSearchResult() : index(0), distanceSquared(0) {}

///
PhotonMap::MaxDistanceSearchResult PhotonMap::findMaxDistancePhotonIndex(const std::vector<int> & photonIndices, const Eigen::Vector3f & intersection) {
    MaxDistanceSearchResult result;

    result.index = -1;
    result.distanceSquared = -std::numeric_limits<float>::infinity();
    
    for (int i = 0; i < (int) photonIndices.size(); ++i) {
        const auto & photon = photons[photonIndices[i]];
        float distSqd = (photon.position - intersection).dot(photon.position - intersection);
        
        if (distSqd > result.distanceSquared) {
            result.distanceSquared = distSqd;
            result.index =  i;
        }
    }
    
    return result;
}

///
float PhotonMap::gaussianWeight(float distSqrd, float radius) {
    static const float oneOverSqrtTwoPi = 0.3989422804f;
    
    float sigma = radius/3.0;
    return (oneOverSqrtTwoPi / sigma) * exp( - (distSqrd) / (2.0 * sigma * sigma) );
}

///
float PhotonMap::gaussianWeightJensen(float distSqrd, float radius) {
    static const float alpha = 0.918f;
    static const float beta  = 1.953f;

    return alpha * (1.0f - (1.0f - exp(-beta * distSqrd / (2.0f * radius * radius))) / (1.0f - std::exp(-beta))) ;
}

///
void PhotonMap::buildSpatialHash() {
    // calculate hash value for each photon.
    mapPhotonsToGrid();


    // sort photons based on hash ID
    std::sort(photons.begin(), photons.end(), [&](const JensenPhoton & lhs, const JensenPhoton & rhs) {
        return gridIndices[getCellIndexHash(lhs.position)] < gridIndices[getCellIndexHash(rhs.position)];
    });

    gridFirstPhotonIndices.clear();
    gridFirstPhotonIndices.resize(photons.size(), -1);
    
    // calculate starting photon index for each hashID
    computeGridFirstPhotons();
}

///
void PhotonMap::mapPhotonsToGrid() {
    gridIndices.clear();
    gridIndices.resize(photons.size(), -1);
    
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
void PhotonMap::computeGridFirstPhotons() {
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

///
RGBf PhotonMap::gatherPhotons(
    int maxNumPhotonsToGather,
    int intersectedGeomId,
    const Eigen::Vector3f & intersection,
    const Eigen::Vector3f & normal,
    float flux) {

    RGBf accumColor = RGBf::Zero();
    auto gridIndex = getCellIndex(intersection);
    int px = gridIndex.x(), py = gridIndex.y(), pz = gridIndex.z();
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < xdim
     && py >= 0 && py < ydim
     && pz >= 0 && pz < zdim) {
        
        float maxRadiusSqd = -1.0f;
        
        // Find photons in neighboring cells
        std::vector<int> neighborPhotons;
        
        for (int i = std::max(0, px - spacing); i < std::min(xdim, px+spacing+1); ++i) {
            for (int j = std::max(0, py - spacing); j < std::min(ydim, py+spacing+1); ++j) {
                for (int k = std::max(0, pz - spacing); k < std::min(zdim, pz+spacing+1); ++k) {
                    
                    int gridIndex = photonHash(i, j, k);
                    // find the index of the first photon in the cell
                    int firstPhotonIndex = gridFirstPhotonIndices[gridIndex];
                    if (firstPhotonIndex != -1) {
                        int pi = firstPhotonIndex;
                        while (pi < photons.size() && gridIndices[pi] == gridIndex) {
                            const auto & p = photons[pi];
                            // Check if the photon is on the same geometry as the intersection
                            if (p.flags.geometryIndex == (uint16_t) intersectedGeomId) {
                                // We only store K photons. If there are less than K photons stored in the array, add the current photon to the array
                                if (neighborPhotons.size() < maxNumPhotonsToGather) {
                                    neighborPhotons.push_back(pi);
                                    float distSqd = (p.position - intersection).dot(p.position - intersection);
                                    if (distSqd > maxRadiusSqd) {
                                        maxRadiusSqd = distSqd;
                                    }
                                }
                                // If the array is full, find the photon with the largest distance to the intersection. If current photon's distance
                                // to the intersection is smaller, replace the photon with the largest distance with the current photon
                                else {
                                    auto searchResult = findMaxDistancePhotonIndex(neighborPhotons, intersection);
                                    float distSqd = (p.position - intersection).dot(p.position - intersection);
                                    if (distSqd < searchResult.distanceSquared) {
                                        neighborPhotons[searchResult.index] = pi;
                                        if (distSqd > maxRadiusSqd
                                         || fabs(maxRadiusSqd - searchResult.distanceSquared) < epsilon) {
                                            maxRadiusSqd = distSqd;
                                        }
                                    }
                                }
                            }
                            pi++;
                        }
                    }
                }
            }
        }
        
        
        
        // if there are more than 0 photons in the neighborhood, find sqr of maxDistanceSquared
        float maxRadius = maxRadiusSqd> 0.0 ? sqrt(maxRadiusSqd) : -1.0f;
        // Accumulate radiance of the K nearest photons
        for (int i=0; i < neighborPhotons.size(); ++i) {
            const auto & p = photons[neighborPhotons[i]];
            float photonDistanceSqd = (intersection - p.position).dot(intersection - p.position);
            accumColor += gaussianWeight(photonDistanceSqd, maxRadius) * std::max(0.0f, normal.dot(-p.incomingDirection.vector())) * rgbe2rgb(p.energy);
        }
    }
    return accumColor * flux;
}