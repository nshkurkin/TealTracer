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

template <typename PixelFieldType>
struct Image {

    ///
    typedef Eigen::Matrix<PixelFieldType, 4, 1> Vector4;

    std::vector<Vector4> pixels;
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
    void setDimensions(int width, int height, const Vector4 & fill = Vector4(100, 100, 100, 255)) {
        this->width = width;
        this->height = height;
        
        pixels.resize(width * height, fill);
    }
    
    ///
    Vector4 & pixel(int x, int y) {
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
    
    //
    size_t dataSize() {
        return sizeof(PixelFieldType) * size_t(4 * width * height);
    }
};

#endif /* Image_hpp */
