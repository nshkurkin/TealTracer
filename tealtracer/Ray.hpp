//
//  Ray.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef Ray_hpp
#define Ray_hpp

#include <Eigen/Dense>

///
struct Ray {
    Eigen::Vector3f origin;
    Eigen::Vector3f direction;
};

///
struct RayIntersectionResult {
    bool intersected;
    float timeOfIntersection;
    Ray ray;
    Eigen::Vector3f surfaceNormal;
    
    ///
    RayIntersectionResult() : intersected(false), timeOfIntersection(0) {}
    
    ///
    Eigen::Vector3f locationOfIntersection() const {
        return ray.origin + (ray.direction * timeOfIntersection);
    }
    
    ///
    Eigen::Vector3f outgoingDirection() const {
        /// http://www.cosinekitty.com/raytrace/chapter10_reflection.html
        return ray.direction - 2.0 * (ray.direction.dot(surfaceNormal) * surfaceNormal);
    }
};

#endif /* Ray_hpp */
