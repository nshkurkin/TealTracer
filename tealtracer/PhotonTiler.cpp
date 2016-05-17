//
//  PhotonTiler.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/12/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PhotonTiler.hpp"

///
PhotonTiler::PhotonTiler() {

}
    
///
PhotonTiler::Plane::Plane() : normal(0,0,0), distance(0) {}

///
PhotonTiler::Plane::Plane(const Eigen::Vector3f & normal, float distance) : normal(normal), distance(distance) {}

///
float
PhotonTiler::Plane::distanceToPoint(const Eigen::Vector3f & point) {
    return normal.dot(point - distance*normal);
}

///
PhotonTiler::Frustum::Frustum() {}

///
bool
PhotonTiler::Frustum::intersectsOrContainsSphere(const Eigen::Vector3f & sphereCenter, float sphereRadius) {
    bool sphereCompletelyOutside = false;
    for (size_t planeItr = 0; planeItr < planes.size() && !sphereCompletelyOutside; planeItr++) {
        float distance = planes[planeItr].distanceToPoint(sphereCenter);
        sphereCompletelyOutside = (distance + sphereRadius < 0);
    }
    
    return !sphereCompletelyOutside;
}

///
void
PhotonTiler::Frustum::setPlanesFromRayCast(const Eigen::Vector3f & tileProjectionOrigin, const Eigen::Vector3f & tileCenter, const Eigen::Vector3f & camera_up, const Eigen::Vector3f & camera_right, float boxWidth, float boxHeight) {
    
    planes.clear();
    
    std::vector<Eigen::Vector3f> offsets = {
        (boxWidth/2.0f)*camera_right + (boxHeight/2.0f)*camera_up,
        (boxWidth/2.0f)*camera_right - (boxHeight/2.0f)*camera_up,
        -(boxWidth/2.0f)*camera_right - (boxHeight/2.0f)*camera_up,
        -(boxWidth/2.0f)*camera_right + (boxHeight/2.0f)*camera_up
    };
    
    for (size_t itr = 0; itr < offsets.size(); itr++) {
        Eigen::Vector3f corner = tileCenter + offsets[itr], nextCorner = tileCenter + offsets[(itr+1) % offsets.size()];
        Eigen::Vector3f originToCorner = corner - tileProjectionOrigin;
        Eigen::Vector3f originToNextCorner = nextCorner - tileProjectionOrigin;
        Eigen::Vector3f planeNormal = originToCorner.cross(originToNextCorner).normalized();
        
//                assert(planeNormal.dot(tileCenter - corner) > 0);
        planes.push_back(Plane(planeNormal, planeNormal.dot(corner)));
        
//                float distToCorner = planes[itr].distanceToPoint(corner);
//                float distToNextCorner = planes[itr].distanceToPoint(nextCorner);
//                float distToOrigin = planes[itr].distanceToPoint(tileProjectionOrigin);
//                float distToTileCenter = planes[itr].distanceToPoint(tileCenter);
//                
//                assert(distToCorner > -0.001 && distToCorner < 0.001);
//                assert(distToNextCorner > -0.001 && distToNextCorner < 0.001);
//                assert(distToOrigin > -0.001 && distToOrigin < 0.001);
//                assert(distToTileCenter > 0.01);
    }
}

///
PhotonTiler::Tile::Tile() {}

///
void
PhotonTiler::generateTiles(
    const int imageWidth, const int imageHeight,
    const int tileWidth, const int tileHeight,
    const Eigen::Vector3f & cameraPosition,
    const FrenetFrame & viewFrame) {
    
    tiles.clear();
    tilePhotons.clear();
    
    for (int py = 0; py < imageHeight; py += tileHeight) {
        for (int px = 0; px < imageWidth; px += tileWidth) {
            Tile tile = Tile();
            Eigen::Vector3f pixelOffset = (viewFrame.forward - 0.5*viewFrame.up - 0.5*viewFrame.right + viewFrame.right*(0.5+(double)(px+tileWidth/2))/(double)imageWidth + viewFrame.up*(0.5+(double)(py + tileHeight/2))/(double)imageHeight);
            Eigen::Vector3f tileCenter = cameraPosition + pixelOffset;
            
            tile.frustum.setPlanesFromRayCast(cameraPosition, tileCenter, viewFrame.up, viewFrame.right, ((double)tileWidth)/((double)imageWidth), ((double)tileHeight)/((double)imageHeight));
            
            tiles.push_back(tile);
        }
    }
}

///
int
PhotonTiler::tileIndexForPixel(
    const int imageWidth, const int imageHeight,
    const int tileWidth, const int tileHeight,
    const int px, const int py) const {
    
    return (px/tileWidth) + (py/tileHeight)*(imageWidth/tileWidth);
}

///
void
PhotonTiler::buildMap(float photonEffectRadius) {
    int numberOfTiles = (int) tiles.size();
    std::vector<int> photonCount(numberOfTiles, 0), nextPhotonIndex(numberOfTiles, 0);
    tilePhotons = std::vector<std::vector<JensenPhoton>>(numberOfTiles, std::vector<JensenPhoton>());
    
    /// Counting pass
    for (size_t photonItr = 0; photonItr < photons.size(); photonItr++) {
        for (size_t tileItr = 0; tileItr < tiles.size(); tileItr++) {
            if (tiles[tileItr].frustum.intersectsOrContainsSphere(photons[photonItr].position, photonEffectRadius)) {
                photonCount[tileItr] += 1;
            }
        }
    }
    
    /// Allocation pass
    for (size_t tileItr = 0; tileItr < tiles.size(); tileItr++) {
        tilePhotons[tileItr] = std::vector<JensenPhoton>(photonCount[tileItr], JensenPhoton());
    }
    
    /// Copy pass
    for (size_t photonItr = 0; photonItr < photons.size(); photonItr++) {
        for (size_t tileItr = 0; tileItr < tiles.size(); tileItr++) {
            if (tiles[tileItr].frustum.intersectsOrContainsSphere(photons[photonItr].position, photonEffectRadius)) {
            
                int tilePhotonIdx = nextPhotonIndex[tileItr]++;
                tilePhotons[tileItr][tilePhotonIdx] = photons[photonItr];
            }
        }
    }
}
