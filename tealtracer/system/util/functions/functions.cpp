///
/// functions.cpp
/// -------------
/// Nikolai Shkurkin
/// Utility Library
///

#include "functions.h"

using namespace util;

Mat4f util::getOrtho2D(float left, float right, float bottom, float top, float zNear,
 float zFar) {
    Mat4f M = Mat4f::Zero();
    
    M(0,0) = 2.0f / (right - left);
    M(1,1) = 2.0f / (top - bottom);
    M(2,2) = -2.0f / (zFar - zNear);
    M(0,3) = - (right + left) / (right - left);
    M(1,3) = - (top + bottom) / (top - bottom);
    M(2,3) = - (zFar + zNear) / (zFar - zNear);
    M(3,3) = 1.0f;
    
    return M;
}

Mat4f util::getRotateX(float radians) {
    Mat4f M = Mat4f::Identity();
    
    float s = std::sin(radians);
    float c = std::cos(radians);
    M(1,1) = c;
    M(1,2) = -s;
    M(2,1) = s;
    M(2,2) = c;
    return M;
}

Mat4f util::getRotateY(float radians) {
    Mat4f M = Mat4f::Identity();
    
    float s = std::sin(radians);
    float c = std::cos(radians);
    M(0,0) = c;
    M(0,2) = s;
    M(2,0) = -s;
    M(2,2) = c;
    return M;
}

Vec3f util::getXFromZ(Vec3f zaxis) {
    return util::getPerp<float>(zaxis);
}
Vec3f util::getYFromZ(Vec3f zaxis) {
    return zaxis.cross(util::getPerp<float>(zaxis));
}
Vec3f util::getXFromY(Vec3f yaxis) {
    return util::getXFromZ(yaxis);
}
Vec3f util::getZFromY(Vec3f yaxis) {
    return -util::getYFromZ(yaxis);
}
Vec3f util::getYFromX(Vec3f xaxis) {
    return util::getYFromZ(xaxis);
}
Vec3f util::getZFromX(Vec3f xaxis) {
    return -util::getXFromZ(xaxis);
}

Vec3f util::reflect(Vec3f incoming, Vec3f normal) {
    Vec3f n = normal / normal.norm(); // unit length
    
    return incoming - 2 * n.dot(incoming) * n;
}

float util::easeIn(float t, float b, float c, float d) {
    return -c * (sqrt(1 - (t/=d)*t) - 1) + b;
}

Vec3f util::safelyNormalize(Vec3f vec, Vec3f vecIfZero) {
    if (vec.norm() > 0.001)
        return vec.normalized();
    else
        return vecIfZero;
}

Mat4f util::getLookAtMatrix(Vec3f eye, Vec3f pos, Vec3f up) {
    Vec3f f(util::safelyNormalize(pos - eye, Vec3f(1, 0, 0)));
    Vec3f s(util::safelyNormalize(f.cross(up.normalized()), Vec3f(1, 0, 0)));
    Vec3f u(s.cross(f));
    
    Mat4f lookAtMat(Mat4f::Identity());
    
    lookAtMat.block<1, 3>(0, 0) = s;
    lookAtMat.block<1, 3>(1, 0) = u;
    lookAtMat.block<1, 3>(2, 0) = -f;
    return lookAtMat;
}

float util::clampToRange(float val, float a, float b) {
    return util::clampToRange(val, 0, a, b);
}

float util::clampToRange(float val, float eps, float a, float b) {
    if (a < b) {
        if (val - eps <= a)
            return a + eps;
        else if (val + eps >= b)
            return b - eps;
        else
            return val;
    }
    else {
        if (val - eps <= b)
            return b + eps;
        else if (val + eps >= a)
            return a - eps;
        else
            return val;
    }
}

