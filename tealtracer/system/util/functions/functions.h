///
/// functions.h
/// -----------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_functions__
#define ____util_functions__

#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

#include <src/util/util_base.h>
#include <src/util//util_types.h>
#include <src/util/data/data.h>

#include <stb/stb.h>

#include "Equation.h"
#include "PerlinNoise.h"
#include "Random.h"

namespace util {
    
    /// Returns a vector guaranteed to be perpendicular to `vec`. By convention,
    /// if you give it the z-axis, it will return the x-axis.
    template <typename T>
    typename Types<T>::Vec3 getPerp(typename Types<T>::Vec3 vec) {
        typedef typename Types<T>::Vec3 Vec3;
        
        Vec3 toRet;
        if (vec.norm() < T(0.0001))
            vec.x() = T(1);
        T cosTheta = vec.normalized().dot(Vec3(T(0), T(0), T(1)));
        T sinTheta = sqrt(T(1.0) - cosTheta * cosTheta);
        
        toRet.x() = cosTheta + vec.y() * vec.y() * (T(1.0) - cosTheta);
        toRet.y() =  - vec.x() * vec.y() * (T(1.0) - cosTheta);
        toRet.z() =  - vec.x() * sinTheta;
        
        return toRet;
    }
    
    Mat4f getOrtho2D(float left, float right, float bottom,
                     float top, float zNear, float zFar);
    Mat4f getRotateX(float radians);
    Mat4f getRotateY(float radians);
    
    Vec3f getXFromZ(Vec3f zaxis);
    Vec3f getYFromZ(Vec3f zaxis);
    Vec3f getXFromY(Vec3f yaxis);
    Vec3f getZFromY(Vec3f yaxis);
    Vec3f getYFromX(Vec3f xaxis);
    Vec3f getZFromX(Vec3f xaxis);
    
    template <typename T>
    std::string to_string(const T &n) {
        #ifdef __WINDOWS__
            std::ostringstream stm;
            stm << n;
            return stm.str();
        #else
            return std::to_string(n);
        #endif
    }
    
    /// Reflect incoming across normal vector
    Vec3f reflect(Vec3f incoming, Vec3f normal);
    
    float easeIn(float t, float b, float c, float d);
    
    /// Linear interpolates `start` to `end` by `a`.
    template <typename Scalable, typename Scalar>
    Scalable lerp(Scalar a, Scalable start, Scalable end) {
        return (Scalar(1) - a) * start + a * end;
    }
    
    /// Spherically linearly interpolates two quaternions by `a`.
    template <typename T>
    Eigen::Quaternion<T> slerp(T a, Eigen::Quaternion<T> start,
     Eigen::Quaternion<T> end) {
        return start.slerp(a, end);
    }
    
    /// Normalizes only if the norm() of the vector is non-zero. If the vector
    /// has a norm close to (0, 0, 0), then `vecIfZero` is used. If you always
    /// want the vector return to be always non-zero, then make vecIfZero
    /// some non-zero vector like (1, 0, 0).
    Vec3f safelyNormalize(Vec3f vec, Vec3f vecIfZero = Vec3f(0, 0, 0));
    
    /// Solves the system defined by Ax = b using SVD decomposition.
    template <typename T, int N, int M>
    Eigen::Matrix<T, M, 1> solve(const Eigen::Matrix<T, N, M> & A,
     const Eigen::Matrix<T, N, 1> b) {
        Eigen::Matrix<T, M, 1> x;
        return A.colPivHouseholderQr().solve(b);
    }
    
    
}

namespace util {
    /// Returns the matrix that represents the rotation needed from `eye` to
    /// "look at" `pos` with `up` as the up direction.
    Mat4f getLookAtMatrix(Vec3f eye, Vec3f pos, Vec3f up);
}

namespace util {
    template <typename T>
    bool valueIsWithinRange(T val, T a, T b) {
        util::valueIsWithinRange<T>(val, T(0), a, b);
    }
    template <typename T>
    bool valueIsWithinRange(T val, T eps, T a, T b) {
        if (a < b)
            return a <= val - eps && val + eps <= b;
        else
            return b <= val - eps && val + eps <= a;
    }
    
    float clampToRange(float val, float a, float b);
    float clampToRange(float val, float eps, float a, float b);
    
    template <typename T, int N>
    Eigen::Matrix<T, N, 1> clamp(Eigen::Matrix<T, N, 1> vec, float a, float b) {
        for (int i = 0; i < N; i++)
            vec[i] = util::clampToRange(vec[i], a, b);
        return vec;
    }
}

namespace util {

    template <typename T, typename O>
    inline void addto_vector(std::vector<T> & outVec, O t) {
        outVec.push_back(T(t));
    }
    
    template <typename T, typename O, typename... Args>
    inline void addto_vector(std::vector<T> & outVec, O t, Args... args) {
        outVec.push_back(T(t));
        util::addto_vector(outVec, args...);
    }
    
    template <typename T, typename... Args>
    std::vector<T> make_vector(T t, Args... args) {
        std::vector<T> vector;
        util::addto_vector(vector, t, args...);
        return vector;
    }

}

namespace util {
    
    template <int kNumComps>
    bool saveImageToPNGFile(util::Image< Eigen::Matrix<uint8_t, kNumComps, 1> >
     img, std::string fileName) {
        if (img.width > 0 && img.height > 0) {
            /// stbi_write writes left-to-write and top-to-bottom. So we need
            /// to flip vertically in order to preserve the image
            img.flipVertically();
            return stbi_write_png(fileName.c_str(), img.width, img.height,
             kNumComps, &img(0, 0).x(), kNumComps * img.width) != 0;
        }
        else
            return false;
    }
    
    template <int kNumComps>
    util::Image< Eigen::Matrix<uint8_t, kNumComps, 1> >
     loadImageFromFile(std::string fileName,
     Eigen::Matrix<uint8_t, kNumComps, 1> filler
     = Eigen::Matrix<uint8_t, kNumComps, 1>::Zero()) {
        typedef util::Image< Eigen::Matrix<uint8_t, kNumComps, 1> > ImgType;
        int w, h, ncomps;
        uint8_t *data = stbi_load(fileName.c_str(), &w, &h, &ncomps, 0);
        typename ImgType::CollectionType parsedData;
        
        if (data == NULL || ncomps == 0) {
            UTIL_LOG(fileName + " not found.");
            UTIL_WHAT_IS(ncomps);
            exit(1);
        }
        
        int which = -ncomps;
        while ((which += ncomps) < (w * h * ncomps)) {
            int whichItem = 0, offset = 0;
            typename ImgType::DataType pixel = filler;
            while (whichItem < ncomps && whichItem < kNumComps)
                pixel[whichItem++] = data[which + (offset++)];
            parsedData.push_back(pixel);
        }
        
        ImgType img(w, h, parsedData);
        stbi_image_free(data);
        return img;
    }
    
}

#endif // ____util_functions__
