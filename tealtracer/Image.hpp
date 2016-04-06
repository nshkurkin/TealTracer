//
//  Image.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/5/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef Image_hpp
#define Image_hpp

#include <vector>
#include <Eigen/Dense>

struct Image {

    ///
    typedef Eigen::Matrix<uint8_t, 4, 1> Vector4ub;

    std::vector<Vector4ub> pixels;
    int width;
    int height;
    
    ///
    Image() {
        setDimensions(0, 0);
    }
    
    ///
    Image(int width, int height) {
        setDimensions(width, height);
    }
    
    ///
    void setDimensions(int width, int height, const Vector4ub & fill = Vector4ub(100, 100, 100, 255)) {
        this->width = width;
        this->height = height;
        
        pixels.resize(width * height, fill);
    }
    
    ///
    Vector4ub & pixel(int x, int y) {
        return pixels[x + width * y];
    }
    
    ///
    void * dataPtr() {
        if (pixels.size() > 0) {
            return &pixels[0];
        }
        else {
            return nullptr;
        }
    }
};

#endif /* Image_hpp */
