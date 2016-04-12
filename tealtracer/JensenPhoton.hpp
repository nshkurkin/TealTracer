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
float frexp(float val, int32_t * ptr);
///
float modulus(const float & lhs, const float & rhs);

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
WardRGBE rgb2rgbe(const RGBf & rgb);

/// Converts from rgbe to float pixels
///
/// NOTE: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels
///       in the range [0,1] to map back into the range [0,1].
///
/// /// From http://www.graphics.cornell.edu/%7Ebjw/rgbe/rgbe.c
RGBf rgbe2rgb(WardRGBE rgbe);

/// Represents a normalized 3-vector which has a data foot-print less than or
/// equal to the foot-print of 3 high-precision floating point types. A CNV3 is
/// composed of two `DiscretizationType` angles that represent spherical
/// coordinates (radius = 1).
struct CompressedNormalVector3 {

    /// The discretization type for angles from [0,2pi]
    typedef uint8_t DiscretizationType;

    /// Returns the result of uncompressing a UInt8 into an angle that lies in the
    /// interval [0,2pi].
    static float uncompressAngleValue(const DiscretizationType & compressedValue);

    /// Returns the result of compressing some angle in the interval [0,2pi] as one 
    /// of UInt8.max discretized chunks.
    static DiscretizationType compressAngleValue(const float & value);


private:
    /// The compressed angle of rotation around the z-axis of the vector. Angles 
    /// are compressed as a discretization of [0,2pi] into UInt8.max chunks.
    DiscretizationType theta_;
    
    /// The rotation around the y-axis, after applying `theta` of the vector.
    /// Angles are compressed as a discretization of [0,2pi] into UInt8.max chunks.
    DiscretizationType phi_;

public:
    /// The raw compressed theta angle value (read-only)
    const DiscretizationType & compressedTheta() const;
    
    /// The raw compressed phi angle value (read-only)
    const DiscretizationType &  compressedPhi() const;
    
    /// Creates a compressed version of [0,0,1]
    CompressedNormalVector3();
    
    /// Creates a compressed version of `vector`, assuming that vector is 
    /// normalized.
    CompressedNormalVector3(const Eigen::Vector3f & vector);
    
    /// The computed, uncompressed version of `theta`, the angle of rotation 
    /// around the z-axis
    float theta() const;
    void setTheta(float value);
    
    /// The computed, uncompressed version of `phi`, the rotation around the 
    /// y-axis, after applying `theta` of the vector.
    float phi() const;
    void setPhi(float value);
    
    /// The computed incoming direction vector for this photon.
    ///
    /// Formulae for converting Cartesian <-> Spherical taken from
    /// http://mathworld.wolfram.com/SphericalCoordinates.html
    Eigen::Vector3f vector() const;
    
    ///
    void setVector(const Eigen::Vector3f & vector);
};

#include "stl_extensions.hpp"

/// A compact photon that stores various information about direction, positions,
/// energy, and other potentially useful data (i.e. flags)
///
/// Taken from http://graphics.ucsd.edu/~henrik/papers/rendering_caustics/rendering_caustics_gi96.pdf
///     Referenced from: https://sites.fas.harvard.edu/~cs278/papers/pmap.pdf
struct JensenPhoton {

    packed_struct Flags {
        bool shadow: 1;
        bool reflected: 1;
        uint16_t geometryIndex: 14;
        
        Flags();
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
    JensenPhoton();
    
    /// Creates a photon given non-sparse format.
    JensenPhoton(const Eigen::Vector3f & position, const Eigen::Vector3f & incomingDirection, const RGBf & energy, bool shadow, bool reflected, uint16_t geometryIndex);
};

#endif /* JensenPhoton_hpp */
