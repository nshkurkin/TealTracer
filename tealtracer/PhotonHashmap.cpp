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
PhotonHashmap::findMaxDistancePhotonIndex(const std::vector<PhotonMap::PhotonIndexInfo> & photonIndices) {
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

    //
    for (int index = 0; index < photons.size(); index++) {
        
        // First one always has to be stored
        if (index == 0 && gridIndices[index] != -1) {
            assert(gridIndices[index] < gridFirstPhotonIndices.size());
            gridFirstPhotonIndices[gridIndices[index]] = index;
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
void
PhotonHashmap::gatherClosestPhotonsForGridIndex(
    const int maxNumPhotonsToGather,
    const float maxPhotonDistance,
    const Eigen::Vector3f & intersection,
    ///
    int i, int j, int k,
    
    // output
    std::vector<PhotonIndexInfo> & neighborPhotons,
    float * maxRadiusSqd
) {

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
                    *maxRadiusSqd = std::max<float>(distSqd, *maxRadiusSqd);
                }
            }
            // If the array is full, find the photon with the largest distance to the intersection. If current photon's distance
            // to the intersection is smaller, replace the photon with the largest distance with the current photon
            else {
                auto index = findMaxDistancePhotonIndex(neighborPhotons);
                auto searchResult = neighborPhotons[index];
                if (distSqd < searchResult.squareDistance) {
                    neighborPhotons[index].index = pi;
                    neighborPhotons[index].squareDistance = distSqd;
                    if (distSqd > *maxRadiusSqd
                     || fabs(*maxRadiusSqd - searchResult.squareDistance) < epsilon) {
                        *maxRadiusSqd = distSqd;
                    }
                }
            }
            pi++;
        }
    }

}

///
std::vector<PhotonMap::PhotonIndexInfo>
PhotonHashmap::gatherPhotonsIndices_v2(
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
        
        float maxRadiusSqd = 0.0f;
        
        for (int i = std::max(0, px - spacing); i < std::min(xdim, px+spacing+1); ++i) {
            for (int j = std::max(0, py - spacing); j < std::min(ydim, py+spacing+1); ++j) {
                for (int k = std::max(0, pz - spacing); k < std::min(zdim, pz+spacing+1); ++k) {
                    
                    gatherClosestPhotonsForGridIndex(maxNumPhotonsToGather, maxPhotonDistance, intersection, i, j, k, neighborPhotons, &maxRadiusSqd);
                }
            }
        }
    }
    return neighborPhotons;
}


///
static bool pointInsideCube(
    const Eigen::Vector3f & point,
    
    const Eigen::Vector3f & cube_start,
    const Eigen::Vector3f & cube_end) {

    return cube_start.x() <= point.x() && point.x() <= cube_end.x()
     && cube_start.y() <= point.y() && point.y() <= cube_end.y()
     && cube_start.z() <= point.z() && point.z() <= cube_end.z();
}

///
static bool sphereInsideCube(
    const Eigen::Vector3f & sphere_position,
    float sphere_radius_sqd,
    
    const Eigen::Vector3f & cube_center,
    float cube_halfwidth_sqd) {

    Eigen::Vector3f cube_start = cube_center + Eigen::Vector3f(-cube_halfwidth_sqd, -cube_halfwidth_sqd, -cube_halfwidth_sqd);
    Eigen::Vector3f cube_end = cube_center + Eigen::Vector3f(cube_halfwidth_sqd, cube_halfwidth_sqd, cube_halfwidth_sqd);

    return pointInsideCube(sphere_position + Eigen::Vector3f(sphere_radius_sqd, sphere_radius_sqd, sphere_radius_sqd), cube_start, cube_end)
     && pointInsideCube(sphere_position + Eigen::Vector3f(-sphere_radius_sqd, -sphere_radius_sqd, -sphere_radius_sqd), cube_start, cube_end);
}

///
Eigen::Vector3f
PhotonHashmap::getCellBoxStart(
    /// which cell
    int i, int j, int k) {
    
    Eigen::Vector3f start;
    
    start(0) = xmin + (cellsize * (float) i);
    start(1) = ymin + (cellsize * (float) j);
    start(2) = zmin + (cellsize * (float) k);

    return start;
}

///
std::vector<PhotonMap::PhotonIndexInfo>
PhotonHashmap::gatherPhotonsIndices(
    int maxNumPhotonsToGather,
    float maxPhotonDistance,
    const Eigen::Vector3f & intersection) {
    
    auto gridIndex = getCellIndex(intersection);
    int px = gridIndex.x(), py = gridIndex.y(), pz = gridIndex.z();
    std::vector<PhotonIndexInfo> neighborPhotons;
    
    /// Only consider intersections within the grid
    if (px >= 0 && px < xdim
     && py >= 0 && py < ydim
     && pz >= 0 && pz < zdim
     && gridFirstPhotonIndices.size() > 0) {
        
        float maxRadiusSqd = 0.0f;
        
        /// Find initial set of photons
        gatherClosestPhotonsForGridIndex(maxNumPhotonsToGather, maxPhotonDistance, intersection, px, py, pz, neighborPhotons, &maxRadiusSqd);
        
        int innerBoxWidthSize = 1, outerBoxWidthSize = 3;
        float outerBoxWidth = cellsize * (float) outerBoxWidthSize;
        int largestDim = std::max<int>(std::max<int>(xdim, ydim), zdim);
        
        Eigen::Vector3f searchBoxCenter = getCellBoxStart(px, py, pz)
         + 0.5f * Eigen::Vector3f(cellsize, cellsize, cellsize);
        float searchBoxHalfWidthSqd = ((float) (outerBoxWidth * outerBoxWidth)) / 4.0f;
        
        /// Done
        ///     If You have reached the # of photons needed && the search cube encapsulates the sphere
        ///  OR If You have exceeded the max allowed search space
        
        while (!((neighborPhotons.size() == maxNumPhotonsToGather
         && sphereInsideCube(intersection, maxRadiusSqd, searchBoxCenter, searchBoxHalfWidthSqd))
         || (outerBoxWidthSize > largestDim && outerBoxWidth > (2 * spacing + 1)))) {
            for (int counter = 0; counter < outerBoxWidthSize * outerBoxWidthSize * outerBoxWidthSize; counter++) {
                int boxX = (counter % outerBoxWidthSize);
                int boxY = ((counter / outerBoxWidthSize) % outerBoxWidthSize);
                int boxZ = (counter / (outerBoxWidthSize * outerBoxWidthSize));
            
                if (boxX == 1 && boxZ > 0 && (boxZ < outerBoxWidthSize - 1) && boxY > 0 && (boxY < outerBoxWidthSize - 1)) {
                    counter += innerBoxWidthSize;
                }
                
                int bi = (counter % outerBoxWidthSize) - (outerBoxWidthSize / 2);
                int bj = ((counter / outerBoxWidthSize) % outerBoxWidthSize) - (outerBoxWidthSize / 2);
                int bk = (counter / (outerBoxWidthSize * outerBoxWidthSize)) - (outerBoxWidthSize / 2);
                
                int i = std::min<int>(std::max<int>(0, px + bi), xdim - 1);
                int j = std::min<int>(std::max<int>(0, py + bj), ydim - 1);
                int k = std::min<int>(std::max<int>(0, pz + bk), zdim - 1);
                
                if (i == (px + bi) && j == (py + bj) && k == (pz + bk)) {
                    gatherClosestPhotonsForGridIndex(maxNumPhotonsToGather, maxPhotonDistance, intersection, i, j, k, neighborPhotons, &maxRadiusSqd);
                }
            }
        
            outerBoxWidthSize += 2;
            innerBoxWidthSize += 2;
            
            outerBoxWidth = cellsize * (float) outerBoxWidthSize;
            searchBoxHalfWidthSqd = ((float) (outerBoxWidth * outerBoxWidth)) / 4.0f;
        }
    }
    
    return neighborPhotons;
}
