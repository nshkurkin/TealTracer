//
//  BRDF.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/13/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef BRDF_hpp
#define BRDF_hpp

#include <Eigen/Dense>
#include "JensenPhoton.hpp"
#include "PovraySceneElement.hpp"

///
class BRDF {
public:
    PovrayPigment pigment;
    PovrayFinish finish;

    BRDF() {}
    BRDF(const PovrayFinish & finish) : finish(finish) {}
    virtual ~BRDF() {}
    
    ///
    virtual RGBf computeColor(const RGBf & source, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const Eigen::Vector3f & surfaceNormal) = 0;
};

///
class BlinnPhongBRDF : public BRDF {
public:
    virtual ~BlinnPhongBRDF() {}
    
    ///
    virtual RGBf computeColor(const RGBf & source, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const Eigen::Vector3f & surfaceNormal) {
        
        RGBf output;
        
        float multiplier = 0.0;
        
        multiplier += finish.ambient;
        multiplier += finish.diffuse * std::max<float>(0, surfaceNormal.dot(toLight));
        
//        if (finish.roughness > 0.001) {
//            Eigen::Vector3f halfwayVector = (toLight + toViewer).normalized();
//            multiplier += finish.specular * std::pow(std::max<float>(0, surfaceNormal.dot(halfwayVector)), 1.0 / finish.roughness);
//        }
        
        output = pigment.color.block<3,1>(0,0) * multiplier;
        
        output.x() *= source.x();
        output.y() *= source.y();
        output.z() *= source.z();
        
        return output;
    }
};

class OrenNayarBRDF : public BRDF {
public:

    virtual ~OrenNayarBRDF() {}
    
    ///
    virtual RGBf computeColor(const RGBf & source, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const Eigen::Vector3f & surfaceNormal) {
        
        RGBf output;
        float multiplier = 1.0;
        output = pigment.color.block<3,1>(0,0) * multiplier;
        
        return output;
    }
    
};

#endif /* BRDF_hpp */
