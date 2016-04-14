//
//  PhotonKDTree.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/12/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PhotonKDTree_hpp
#define PhotonKDTree_hpp

#include "PhotonMap.hpp"
#include <functional>

struct Range {
    /// "end" is one after the valid index
    int begin, end;
    
    Range() : begin(0), end(0) {}
    Range(int begin, int end) : begin(begin), end(end) {}
};

///
class PhotonKDTree : public PhotonMap {
public:

    PhotonKDTree() {}
    virtual ~PhotonKDTree() {}
    
    /// Call this after filling "photons" with the relevant content.
    virtual void buildMap() {
        transformIntoKDTree();
    }
    
    /// Call this after building the spatial hash.
    ///
    /// NOTE: flux = totalEnergy/(float)numPhotons;
    virtual std::vector<PhotonIndexInfo> gatherPhotonsIndices(
        int maxNumPhotonsToGather,
        float maxPhotonDistance,
        const Eigen::Vector3f & intersection) {
    
        return findClosestNPhotonIndices(intersection, maxNumPhotonsToGather, maxPhotonDistance);
    }

    int rootIdx() const {
        return middleIdx(Range(0,(int) photons.size()));
    }
    /// Returns the last valid index of photons
    int lastIdx() const {
        return (int) photons.size() - 1;
    }
    
    /// Returns the "middle" of a given range, used for finding the nodes of 
    /// the tree. In practice the "middle" is favorited towards the end of
    /// the range.
    static int middleIdx(const Range & range) {
        return range.begin + ((range.end - 1) - range.begin + 1) / 2;
    }
    
    /// Returns the lower partition of a node that overlooks `range`, not including the middle node.
    static Range lowerRange(const Range & range) {
        return Range(range.begin, middleIdx(range));
    }
    
    /// Returns the upper partition of a node that overlooks `range`, not including the middle node.
    static Range upperRange(const Range & range) {
        return Range(middleIdx(range)+1, range.end);
    }
    
    /// Returns the next "axis" after `axis`, which is a value between 0 and
    /// JensenPhoton.kNumPositionComps.
    static int nextAxis(int axis) {
        return (axis+1) % 3;
    }


    /// Curries function that returns the compartor function that compares the
    /// elements of two vectors at row `axis` and returns whether the left is strictly
    /// less than the right.
    static std::function<bool(const Eigen::Vector3f &, const Eigen::Vector3f &)> sparseless(int axis) {
        return [=](const Eigen::Vector3f & lhs, const Eigen::Vector3f & rhs){
            return lhs(axis) < rhs(axis);
        };
    }

    /// Performs an in-place conversion of `photons` into a KD-Tree ordered around
    /// median pivots, organized by each axis in N-space respectively. So the root
    /// is the middle element (photons.count / 2), and the photons before this 
    /// index are descreasing in `x`, and increasing in `x` for the photons after.
    /// The median of the lower half of the array is the next pivot, with the photons
    /// before this indx in descreasing `y`, and increasing in `y` up until the root.
    ///
    /// Implementation gleaned from: http://graphics.ucsd.edu/~henrik/papers/rendering_caustics/rendering_caustics_gi96.pdf
    void transformIntoKDTree() {
        transformIntoKDTree<JensenPhoton>(photons, [](int axis, const JensenPhoton & lhs, const JensenPhoton & rhs) {
            return sparseless(axis)(lhs.position,rhs.position);
        }, nextAxis);
    }
    
    ///
    template <typename T>
    static void transformIntoKDTree(std::vector<T> & values, std::function<bool(int, const T&, const T&)> lessAxis, std::function<int(int)> nextAxis) {
        _transformIntoKDTree<T>(values, lessAxis, nextAxis, 0, (int) values.size() - 1, 0);
    }
    
private:
    /// Private function used to sort only a portion of the array between `startIdx` 
    /// and `endIdx` about `axis`
    template <typename T>
    static void _transformIntoKDTree(std::vector<T> & values, std::function<bool(int, const T&, const T&)> lessAxis, std::function<int(int)> nextAxis, int startIdx, int lastIdx, int axis) {
        if (startIdx < lastIdx) {
            /// Sort this portion of the array by position along the axis
            std::sort(values.begin() + startIdx, values.begin() + lastIdx + 1, [=](const T & lhs, const T & rhs) {
                return lessAxis(axis, lhs, rhs);
            });

            /// From Jensen: the "root node among the data-set as the median element in the direction which represents the largest interval"
            int middleIdx = startIdx + (lastIdx - startIdx) / 2;
            
            _transformIntoKDTree(
                values, lessAxis, nextAxis,
                startIdx, middleIdx - 1,
                nextAxis(axis));
            _transformIntoKDTree(
                values, lessAxis, nextAxis,
                middleIdx + 1, lastIdx,
                nextAxis(axis));
        }
    }

public:
    ///
    typedef PhotonIndexInfo SearchResult;

    /// Consider a point x at which we are interested in the irradiance. Around x we create a sphere. The radius of this sphere is extendeded unitl the sphere contains n photons and has radius r.
    std::vector<SearchResult> findClosestNPhotonIndices(const Eigen::Vector3f & position, int N, float maxSquareDistance) {
         std::vector<SearchResult> candidates, result;
        
        _knnOnPhtonKDTree(position, N, candidates, 0, lastIdx(), 0);
        
        int n = 0;
        while (n < N
         && n < (int) candidates.size()
         && candidates[n].squareDistance <= maxSquareDistance) {
            result.push_back(candidates[n]);
            n++;
        }
        
        return result;
    }
    
private:

    ///
    class CompareSearchResult {
    public:
        bool cmp(const SearchResult & a, const SearchResult & b) {
            return a.squareDistance < b.squareDistance;
        }
        
        bool operator()(const SearchResult & a, const SearchResult & b) {
            return cmp(a, b);
        }
    };

    /// Recursive definition for `findClosestNPhotonIndices`
    ///
    /// Gleaned From: http://web.stanford.edu/class/cs106l/handouts/assignment-3-kdtree.pdf
    void _knnOnPhtonKDTree(const Eigen::Vector3f & position, int N, std::vector<SearchResult> & candidates, int startIdx, int lastIdx, int axis) {
    
        if (startIdx <= lastIdx) {
            auto currIdx = middleIdx(Range(startIdx, lastIdx + 1));
            auto photonPos = photons[currIdx].position;
            Eigen::Vector3f dp = photonPos - position;
            float sqrdist = dp.dot(dp);
            
            auto insertionIndex = std::lower_bound(candidates.begin(), candidates.end(), SearchResult(currIdx, sqrdist), CompareSearchResult());
            candidates.insert(insertionIndex, SearchResult(currIdx, sqrdist));
            
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
            float farthestDist = candidates[candidates.size() - 1].squareDistance;
            if (candidates.size() >= N) {
                farthestDist = candidates[N-1].squareDistance;
            }
            
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

};

#endif /* PhotonKDTree_hpp */
