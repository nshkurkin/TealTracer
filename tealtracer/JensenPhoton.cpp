//
//  JensenPhoton.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "JensenPhoton.hpp"

///
float frexp(float val, int32_t * ptr) {
    return frexpf(val,ptr);
}

///
float modulus(const float & lhs, const float & rhs) {
    float value = lhs;
    if (value < 0) {
        value += rhs * float(std::floor(std::abs(lhs)/rhs) + 1);
    }
    else {
        value = fmod(lhs, rhs);
    }
    return value;
}

///
WardRGBE rgb2rgbe(const RGBf & rgb) {
    float v = 0.0;
    int32_t e = 0;
    WardRGBE rgbe = WardRGBE::Zero();
    
    v = rgb.x();
    if (rgb.y() > v) {v = rgb.y();}
    if (rgb.z() > v) {v = rgb.z();}
    if (v > 1e-32) {
        v = float(frexp(v,&e)) * 256.0/v;
        rgbe(0) = uint8_t(rgb.x() * v);
        rgbe(1) = uint8_t(rgb.y() * v);
        rgbe(2) = uint8_t(rgb.z() * v);
        rgbe(3) = uint8_t(e + 128);
    }
    
    return rgbe;
}

///
RGBf rgbe2rgb(WardRGBE rgbe) {
    float f = 0;
    RGBf rgb = RGBf::Zero();
    
    if (rgbe(3) != 0) {
        f = float(ldexp(1.0,int(rgbe(3))-int(128+8)));
        rgb.x() = float(rgbe(0)) * f;
        rgb.y() = float(rgbe(1)) * f;
        rgb.z() = float(rgbe(2)) * f;
    }
    return rgb;
}

///
float
CompressedNormalVector3::uncompressAngleValue(const CompressedNormalVector3::DiscretizationType & compressedValue) {
    return float(2.0 * M_PI) * float(compressedValue) / float(std::numeric_limits<DiscretizationType>::max());
}

///
CompressedNormalVector3::DiscretizationType
CompressedNormalVector3::compressAngleValue(const float & value) {
    return DiscretizationType(float(std::numeric_limits<DiscretizationType>::max()) * modulus(value, float(2.0 * M_PI)) / float(2.0 * M_PI));
}


///
const CompressedNormalVector3::DiscretizationType &
CompressedNormalVector3::compressedTheta() const {
    return theta_;
}

///
const CompressedNormalVector3::DiscretizationType &
CompressedNormalVector3::compressedPhi() const {
    return phi_;
}

///
CompressedNormalVector3::CompressedNormalVector3() {
    theta_ = 0;
    phi_ = 0;
}

///
CompressedNormalVector3::CompressedNormalVector3(const Eigen::Vector3f & vector) {
    theta_ = 0;
    phi_ = 0;
    setVector(vector);
}

///
float
CompressedNormalVector3::theta() const {
    return uncompressAngleValue(theta_);
}

///
void
CompressedNormalVector3::setTheta(float value) {
    theta_ = compressAngleValue(value);
}

///
float
CompressedNormalVector3::phi() {
    return uncompressAngleValue(phi_);
}

///
void
CompressedNormalVector3::setPhi(float value) {
    phi_ = compressAngleValue(value);
}

///
Eigen::Vector3f
CompressedNormalVector3::vector() const {
    
    float sintheta = float(sin(theta_));
    float costheta = float(cos(theta_));
    float sinphi = float(sin(phi_));
    float cosphi = float(cos(phi_));
    
    return Eigen::Vector3f(costheta * sinphi, sintheta * sinphi, cosphi);
}

///
void
CompressedNormalVector3::setVector(const Eigen::Vector3f & vector) {
    setTheta(float(atan2(double(vector.y()), double(vector.x()))));
    setPhi(float(acos(double(vector.z()))));
}

///
JensenPhoton::Flags::Flags() : shadow(false), reflected(false), materialIndex(0), geometryIndex(0) {

}

///
JensenPhoton::JensenPhoton() {
    position = Eigen::Vector3f::Zero();
    energy = WardRGBE::Zero();
    incomingDirection = CompressedNormalVector3();
    flags = Flags();
}

///
JensenPhoton::JensenPhoton(const Eigen::Vector3f & position, const Eigen::Vector3f & incomingDirection, const RGBf & energy, bool shadow, bool reflected, uint16_t materialIndex) {
    this->position = position;
    this->incomingDirection = CompressedNormalVector3(incomingDirection.normalized());
    this->energy = rgb2rgbe(energy);
    
    this->flags = Flags();
    this->flags.shadow = shadow;
    this->flags.reflected = reflected;
    this->flags.materialIndex = materialIndex;
}

