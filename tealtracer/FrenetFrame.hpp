//
//  FrenetFrame.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef FrenetFrame_hpp
#define FrenetFrame_hpp

#include <Eigen/Dense>

struct FrenetFrame {
    Eigen::Vector3f forward, up, right;
    
    FrenetFrame() : forward(0,0,-1), up(0,1,0), right(1,0,0) {}
    FrenetFrame(const Eigen::Vector3f & forward, const Eigen::Vector3f & up, const Eigen::Vector3f & right) : forward(forward), up(up), right(right) {}
};

#endif /* FrenetFrame_hpp */
