///
/// FrenetFrame.h
/// -------------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_FrenetFrame__
#define ____util_FrenetFrame__

#include <src/util/util_base.h>
#include <src/util/util_types.h>

namespace util {
    
    /// A FrenetFrame represents an othonormal basis of R3 than can be
    /// be transformed to help users keep track of the current axis alignments.
    /// This can be particularly useful for when you need to visualize the
    /// axes of a particular transformation or node of a hierarchy.
    template< typename T >
    struct FrenetFrame {
        DEFINE_EIGEN_TYPES(T)
        
        /// The axes of the othronormal system.
        Vec3 xaxis, yaxis, zaxis;
        
        /// Constructs a FrenetFrame using the default xyz othornormal basis.
        FrenetFrame() : xaxis(T(1), T(0), T(0)), yaxis(T(0), T(1), T(0)),
        zaxis(T(0), T(0), T(1)) {}
        
        /// Construct a custom "basis" (although this is not checked explicitly.
        FrenetFrame(Vec3 xaxis, Vec3 yaxis, Vec3 zaxis) {
            this->xaxis = xaxis; this->yaxis = yaxis; this->zaxis = zaxis;
        }
        
        /// Create a FrenetFrame from another.
        FrenetFrame(const FrenetFrame & frame) {
            this->xaxis = frame.xaxis; this->yaxis = frame.yaxis;
            this->zaxis = frame.zaxis;
        }
        
        /// Rotates the whole frame using an AxisAngle rotation around
        /// `axis` of angle `angle`.
        FrenetFrame rotate(T angle, Vec3 axis) {
            return rotate(AngleAxis(angle, axis));
        }
        
        /// Rotates the whole frame using an AxisAngle rotation.
        FrenetFrame rotate(AngleAxis angleAxis) {
            return rotate(Quat(angleAxis));
        }
        
        /// Rotates the whole frame using the quaternion `quat`.
        FrenetFrame rotate(Quat quat) {
            FrenetFrame toRet(*this);
            toRet.xaxis = quat.matrix() * toRet.xaxis;
            toRet.yaxis = quat.matrix() * toRet.yaxis;
            toRet.zaxis = quat.matrix() * toRet.zaxis;
            return toRet;
        }
        
        /// Gets the quaternion that describes the current x-rotation from
        /// the standard basis.
        Quat xrot() {return Quat::FromTwoVectors(Vec3(T(1), T(0), T(0)), xaxis);}
        
        /// Gets the quaternion that describes the current y-rotation from
        /// the standard basis.
        Quat yrot() {return Quat::FromTwoVectors(Vec3(T(0), T(1), T(0)), yaxis);}
        
        /// Gets the quaternion that describes the current z-rotation from
        /// the standard basis.
        Quat zrot() {return Quat::FromTwoVectors(Vec3(T(0), T(0), T(1)), zaxis);}
        
        /// Transforms `stdVec` into this frame's coordinate space.
        Vec3 toFrame(Vec3 stdVec) {
            Vec3 xp(stdVec.x(), T(0), T(0));
            Vec3 yp(T(0), stdVec.y(), T(0));
            Vec3 zp(T(0), T(0), stdVec.z());
            
            return xrot() * xp + yrot() * yp + zrot() * zp;
        }
        
        /// Returns a description of this frame.
        std::string description() {
            return std::string("(FrenetFrame) {xaxis: " + xaxis + ", yaxis: "
                               + yaxis + ", zaxis: " + zaxis + "}");
        }
    };
}

#endif // ____util_FrenetFrame__
