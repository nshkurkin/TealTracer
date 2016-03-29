///
/// util_types.h
/// ------------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_types__
#define ____util_types__

#include <string>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <Eigen/Dense>

namespace util {
    /// For parsing strings
    typedef std::istringstream StringParser;
}

namespace util {
    template < typename T >
    struct Types {
        typedef Eigen::Matrix<T, 1, 1> Vec1;
        typedef Eigen::Matrix<T, 2, 1> Vec2;
        typedef Eigen::Matrix<T, 3, 1> Vec3;
        typedef Eigen::Matrix<T, 4, 1> Vec4;
        
        typedef Eigen::Quaternion<T> Quat;
        typedef Eigen::AngleAxis<T> AngleAxis;
        
        typedef Eigen::Matrix<T, 1, 2> Mat1;
        typedef Eigen::Matrix<T, 2, 2> Mat2;
        typedef Eigen::Matrix<T, 3, 3> Mat3;
        typedef Eigen::Matrix<T, 4, 4> Mat4;
    };
}

#define DEFINE_EIGEN_TYPES(base) \
    typedef typename util::Types< base >::Vec1 Vec1; \
    typedef typename util::Types< base >::Vec2 Vec2; \
    typedef typename util::Types< base >::Vec3 Vec3; \
    typedef typename util::Types< base >::Vec4 Vec4; \
    typedef typename util::Types< base >::Quat Quat; \
    typedef typename util::Types< base >::AngleAxis AngleAxis; \
    typedef typename util::Types< base >::Mat2 Mat2; \
    typedef typename util::Types< base >::Mat3 Mat3; \
    typedef typename util::Types< base >::Mat4 Mat4; \

#define DEFINE_EIGEN_TYPES_EXT(base, ext) \
    typedef util::Types< base >::Vec1 Vec1 ## ext; \
    typedef util::Types< base >::Vec2 Vec2 ## ext; \
    typedef util::Types< base >::Vec3 Vec3 ## ext; \
    typedef util::Types< base >::Vec4 Vec4 ## ext; \
    typedef util::Types< base >::Quat Quat ## ext; \
    typedef util::Types< base >::AngleAxis AngleAxis ## ext; \
    typedef util::Types< base >::Mat2 Mat2 ## ext; \
    typedef util::Types< base >::Mat3 Mat3 ## ext; \
    typedef util::Types< base >::Mat4 Mat4 ## ext; \

namespace util {
    ///
    DEFINE_EIGEN_TYPES_EXT(float, f)
    DEFINE_EIGEN_TYPES_EXT(double, d)
    
    ///
    DEFINE_EIGEN_TYPES_EXT(unsigned char, ub)
    DEFINE_EIGEN_TYPES_EXT(char, b)
    DEFINE_EIGEN_TYPES_EXT(unsigned short, uh)
    DEFINE_EIGEN_TYPES_EXT(short, h)
    DEFINE_EIGEN_TYPES_EXT(unsigned int, ui)
    DEFINE_EIGEN_TYPES_EXT(int, i)
}

#endif // ____util_types__
