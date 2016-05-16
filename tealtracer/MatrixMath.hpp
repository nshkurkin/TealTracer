//
//  MatrixMath.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef MatrixMath_hpp
#define MatrixMath_hpp

#include <Eigen/Dense>

Eigen::Matrix4f lookAt(const Eigen::Vector3f & eye, const Eigen::Vector3f & center, const Eigen::Vector3f & up);

#endif /* MatrixMath_hpp */
