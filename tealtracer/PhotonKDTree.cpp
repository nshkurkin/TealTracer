//
//  PhotonKDTree.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/12/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PhotonKDTree.hpp"

///
PhotonKDTree::PhotonKDTree() {}

///
PhotonKDTree::~PhotonKDTree() {}

///
void
PhotonKDTree::buildMap() {
    transformIntoKDTree();
}

///
std::vector<PhotonKDTree::PhotonIndexInfo>
PhotonKDTree::gatherPhotonsIndices(
    int maxNumPhotonsToGather,
    float maxPhotonDistance,
    const Eigen::Vector3f & intersection) {

    return findClosestNPhotonIndices(intersection, maxNumPhotonsToGather, maxPhotonDistance);
}

///
int
PhotonKDTree::rootIdx() const {
    return middleIdx(Range(0,(int) photons.size()));
}

///
int
PhotonKDTree::lastIdx() const {
    return (int) photons.size() - 1;
}

///
int
PhotonKDTree::middleIdx(const Range & range) {
    return range.begin + ((range.end - 1) - range.begin + 1) / 2;
}

///
Range
PhotonKDTree::lowerRange(const Range & range) {
    return Range(range.begin, middleIdx(range));
}

///
Range
PhotonKDTree::upperRange(const Range & range) {
    return Range(middleIdx(range)+1, range.end);
}

///
int
PhotonKDTree::nextAxis(int axis) {
    return (axis+1) % 3;
}

/// 
void
PhotonKDTree::transformIntoKDTree() {
    transformIntoKDTree<JensenPhoton>(photons, [](int axis, const JensenPhoton & lhs, const JensenPhoton & rhs) {
        return sparseless(axis)(lhs.position,rhs.position);
    }, nextAxis);
}

///
std::vector<PhotonKDTree::SearchResult>
PhotonKDTree::findClosestNPhotonIndices(const Eigen::Vector3f & position, int N, float maxSquareDistance) {\
    std::vector<SearchResult> result;
    search_queue candidates;
    
    _knnOnPhtonKDTree(position, N, candidates, 0, lastIdx(), 0);
    
    while (!candidates.empty()) {
        result.push_back(candidates.top());
        candidates.pop();
    }
    
    return result;
}

///
void
PhotonKDTree::_knnOnPhtonKDTree(const Eigen::Vector3f & position, int N, PhotonKDTree::search_queue & candidates, int startIdx, int lastIdx, int axis) {

    if (startIdx <= lastIdx) {
        auto currIdx = middleIdx(Range(startIdx, lastIdx + 1));
        auto photonPos = photons[currIdx].position;
        Eigen::Vector3f dp = photonPos - position;
        float sqrdist = dp.dot(dp);
        
        candidates.push(SearchResult(currIdx, sqrdist));
        if (candidates.size() > N) {
            candidates.pop();
        }
        
        auto searchLeft = [&](){
            auto newRange = lowerRange(Range(startIdx, lastIdx + 1));
            this->_knnOnPhtonKDTree(position, N, candidates, newRange.begin, newRange.end - 1, nextAxis(axis));
        };
        auto searchRight = [&](){
            auto newRange = upperRange(Range(startIdx, lastIdx + 1));
            this->_knnOnPhtonKDTree(position, N, candidates, newRange.begin, newRange.end - 1, nextAxis(axis));
        };
        
        bool searchedLeft = true;
        if (sparseless(axis)(position, photonPos)) {
            searchLeft();
        }
        else {
            searchRight();
            searchedLeft = false;
        }
        
        /// If the candidate hypersphere crosses the splitting plane, then 
        /// look "on the other side of the plane by examining the other subtree"
        float farthestDist = candidates.top().squareDistance;
        float diff = photonPos(axis) - position(axis);
        if (candidates.size() < N || diff * diff < farthestDist) {
            if (searchedLeft) {
                searchRight();
            }
            else {
                searchLeft();
            }
        }
    }
}
