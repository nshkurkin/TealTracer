//
//  JensenPhoton.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/10/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef JensenPhoton_hpp
#define JensenPhoton_hpp

#include <Eigen/Eigen>

/// A rename of `frexpf()`
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

typedef Eigen::Vector3f RGBf;

/// A struct that compacts an `RGB` into 4 bytes for space efficiency purposes.
///
/// From http://www.graphics.cornell.edu/%7Ebjw/rgbe/rgbe.c
///     Referenced From: http://graphics.ucsd.edu/~henrik/papers/rendering_caustics/rendering_caustics_gi96.pdf
///     Referenced From: https://sites.fas.harvard.edu/~cs278/papers/pmap.pdf
typedef Eigen::Matrix<uint8_t, 4, 1> WardRGBE;

/// Converts float pixels to rgbe pixels
///
/// From http://www.graphics.cornell.edu/%7Ebjw/rgbe/rgbe.c
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

/// Converts from rgbe to float pixels
///
/// NOTE: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels
///       in the range [0,1] to map back into the range [0,1].
///
/// /// From http://www.graphics.cornell.edu/%7Ebjw/rgbe/rgbe.c
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

/// Represents a normalized 3-vector which has a data foot-print less than or
/// equal to the foot-print of 3 high-precision floating point types. A CNV3 is
/// composed of two `DiscretizationType` angles that represent spherical
/// coordinates (radius = 1).
struct CompressedNormalVector3 {

    /// The discretization type for angles from [0,2pi]
    typedef uint8_t DiscretizationType;

    /// Returns the result of uncompressing a UInt8 into an angle that lies in the
    /// interval [0,2pi].
    static float uncompressAngleValue(const DiscretizationType & compressedValue) {
        return float(2.0 * M_PI) * float(compressedValue) / float(std::numeric_limits<DiscretizationType>::max());
    }

    /// Returns the result of compressing some angle in the interval [0,2pi] as one 
    /// of UInt8.max discretized chunks.
    static DiscretizationType compressAngleValue(const float & value) {
        return DiscretizationType(float(std::numeric_limits<DiscretizationType>::max()) * modulus(value, float(2.0 * M_PI)) / float(2.0 * M_PI));
    }


private:
    /// The compressed angle of rotation around the z-axis of the vector. Angles 
    /// are compressed as a discretization of [0,2pi] into UInt8.max chunks.
    DiscretizationType theta_;
    
    /// The rotation around the y-axis, after applying `theta` of the vector.
    /// Angles are compressed as a discretization of [0,2pi] into UInt8.max chunks.
    DiscretizationType phi_;

public:
    /// The raw compressed theta angle value (read-only)
    const DiscretizationType & compressedTheta() const {
        return theta_;
    }
    
    /// The raw compressed phi angle value (read-only)
    const DiscretizationType &  compressedPhi() const {
        return phi_;
    }
    
    /// Creates a compressed version of [0,0,1]
    CompressedNormalVector3() {
        theta_ = 0;
        phi_ = 0;
    }
    
    /// Creates a compressed version of `vector`, assuming that vector is 
    /// normalized.
    CompressedNormalVector3(const Eigen::Vector3f & vector) {
        theta_ = 0;
        phi_ = 0;
        setVector(vector);
    }
    
    /// The computed, uncompressed version of `theta`, the angle of rotation 
    /// around the z-axis
    float theta() const {
        return uncompressAngleValue(theta_);
    }
    
    void setTheta(float value) {
        theta_ = compressAngleValue(value);
    }
    
    /// The computed, uncompressed version of `phi`, the rotation around the 
    /// y-axis, after applying `theta` of the vector.
    float phi() {
        return uncompressAngleValue(phi_);
    }
    
    void setPhi(float value) {
        phi_ = compressAngleValue(value);
    }
    
    /// The computed incoming direction vector for this photon.
    ///
    /// Formulae for converting Cartesian <-> Spherical taken from
    /// http://mathworld.wolfram.com/SphericalCoordinates.html
    Eigen::Vector3f vector() const {
        
        float sintheta = float(sin(theta_));
        float costheta = float(cos(theta_));
        float sinphi = float(sin(phi_));
        float cosphi = float(cos(phi_));
        
        return Eigen::Vector3f(costheta * sinphi, sintheta * sinphi, cosphi);
    }
    
    ///
    void setVector(const Eigen::Vector3f & vector) {
        setTheta(float(atan2(double(vector.y()), double(vector.x()))));
        setPhi(float(acos(double(vector.z()))));
    }
};

/// A compact photon that stores various information about direction, positions,
/// energy, and other potentially useful data (i.e. flags)
///
/// Taken from http://graphics.ucsd.edu/~henrik/papers/rendering_caustics/rendering_caustics_gi96.pdf
///     Referenced from: https://sites.fas.harvard.edu/~cs278/papers/pmap.pdf
struct JensenPhoton {

    struct Flags {
        bool shadow: 1;
        bool reflected: 1;
        uint16_t materialIndex: 14;
        
        Flags() : shadow(false), reflected(false), materialIndex(0) {}
    };


    /// The position of this photon/where the photon struck
    Eigen::Vector3f position;
    /// The amount of light energy present in this photon
    WardRGBE energy;
    /// The incoming direction vector for this photon when it struck `position`
    CompressedNormalVector3 incomingDirection;
    /// A collection of bits to represent different characteristics of this photon
    Flags flags;
    
    /// Creates an unclassified photon of no energy positioned at (0,0,0), with a normal along the z-axis
    JensenPhoton() {
        position = Eigen::Vector3f::Zero();
        energy = WardRGBE::Zero();
        incomingDirection = CompressedNormalVector3();
        flags = Flags();
    }
    
    /// Creates a photon given non-sparse format.
    JensenPhoton(const Eigen::Vector3f & position, const Eigen::Vector3f & incomingDirection, const RGBf & energy, bool shadow, bool reflected, uint16_t materialIndex) {
        this->position = position;
        this->incomingDirection = CompressedNormalVector3(incomingDirection.normalized());
        this->energy = rgb2rgbe(energy);
        
        this->flags = Flags();
        this->flags.shadow = shadow;
        this->flags.reflected = reflected;
        this->flags.materialIndex = materialIndex;
    }
};

#endif /* JensenPhoton_hpp */
