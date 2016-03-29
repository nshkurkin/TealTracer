///
/// nsgl_types.h
/// ------------
/// Nikolai Shkurkin
/// NSGL Library
///
/// Synopsis:
///   Provides type definitions from the standard floating point representation,
///   the type of a vec4, as well as commonly used buffers.
///

#ifndef __nsgl_types__
#define __nsgl_types__

#include <string>
#include <Eigen/Dense>

#include "nsgl_base.h"
#include <src/nsgl/gl_interface/buffer/DataBufferObject.h>
#include <src/nsgl/gl_interface/buffer/TextureBufferObject.h>

#include <src/nsgl/gl_interface/shader/Program.h>

namespace nsgl {
    /// The principal floating point type (uses GL's float definition)
    typedef GLfloat Float;
    DEFINE_EIGEN_TYPES_EXT(Float, f)
    /// Index type defintions
    typedef unsigned int UInt;
    DEFINE_EIGEN_TYPES_EXT(UInt, u)
    /// Texture Buffer definitions
    typedef unsigned char UInt8;
    DEFINE_EIGEN_TYPES_EXT(UInt8, ub)
}

namespace nsgl {
    /// Floating point attribute buffer definitions
    typedef DataBufferObject< Float > FloatBufferObject;
    typedef DataBufferObject< Vec2f > Vec2fBufferObject;
    typedef DataBufferObject< Vec3f > Vec3fBufferObject;
    typedef DataBufferObject< Vec4f > Vec4fBufferObject;
    
    /// Represents a collection of floats.
    typedef FloatBufferObject FBO;
    /// Represents a collection of vec2's, usually for texcoords.
    typedef Vec2fBufferObject V2fBO;
    /// Represents a collection of vec3's, usually for positions and normals.
    typedef Vec3fBufferObject V3fBO;
    /// Represents a collection of vec4's, usually for positions, weights,
    /// and bone indices.
    typedef Vec4fBufferObject V4fBO;
}

namespace nsgl {
    /// Index Buffer definitions
    typedef DataBufferObject< UInt > UIntBufferObject;
    typedef DataBufferObject< Vec2u > Vec2uBufferObject;
    typedef DataBufferObject< Vec3u > Vec3uBufferObject;
    typedef DataBufferObject< Vec4u > Vec4uBufferObject;
    
    /// Represents a set of indices for drawing objects. Typically this is all
    /// that you need. For better manipulation of primitive sets (a collection
    /// of contiguous indices), use V*uBO instead.
    typedef UIntBufferObject UIBO;
    /// Represents a set of lines indices of the form [a, b], [c, d], ...
    typedef Vec2uBufferObject V2uBO;
    /// Represents a set of triangle indices of the form [a, b, c], ...
    typedef Vec3uBufferObject V3uBO;
    /// Represents a set of Vec4 indices.
    typedef Vec4uBufferObject V4uBO;
    
    typedef UIntBufferObject PointIndexBufferObject;
    typedef Vec2uBufferObject LineIndexBufferObject;
    typedef Vec3uBufferObject TriangleIndexBufferObject;
}

namespace nsgl {
    /// See `util::RGBAImage`.
    typedef util::RGBImage RGBImage;
    /// See `util::RGBAImage`.
    typedef util::RGBAImage RGBAImage;
    
    /// Reads a file of name `fname` and creates a corresponding `RGBImage`.
    RGBImage rgbImageFromFile(const std::string fname);
    RGBAImage rgbaImageFromFile(const std::string fname);
    
    typedef TextureBufferObject< Vec3ub > RGBBufferObject;
    typedef TextureBufferObject< Vec4ub > RGBABufferObject;
    
    /// Represents a 3-component 2D texture.
    typedef RGBBufferObject  RGBTexture;
    /// Represents a 4-components 2D texture.
    typedef RGBABufferObject RGBATexture;
    
    /// Creates an RGBTexture fomr a correspondign `img` with the texture unit
    /// `textureUnit`.
    RGBTexture rgbTexture(RGBImage & img, GLenum textureUnit);
    RGBTexture rgbTexture(const RGBImage & img, GLenum textureUnit);
    RGBTexture rgbTexture();
    /// Directly reads an RGB image file named `fname` and assigns it to the
    /// texture unit `texUnit`. Equivalent to using `rgbImageFromFile` followed
    /// by `rgbBuffer` with `texUnit`.
    RGBTexture rgbTextureFromFile(const std::string fname, GLenum texUnit);
    
    /// Creates an RGBATexture fomr a correspondign `img` with the texture unit
    /// `textureUnit`.
    RGBATexture rgbaTexture(RGBAImage & img, GLenum textureUnit);
    RGBATexture rgbaTexture(const RGBAImage & img, GLenum textureUnit);
    RGBATexture rgbaTexture();
    /// Directly reads an RGB image file named `fname` and assigns it to the
    /// texture unit `texUnit`. Equivalent to using `rgbImageFromFile` followed
    /// by `rgbBuffer` with `texUnit`.
    RGBATexture rgbaTextureFromFile(const std::string fname, GLenum texUnit);
}

#endif // __nsgl_types__
