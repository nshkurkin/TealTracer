//
//  PovraySceneElements.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PovraySceneElements_hpp
#define PovraySceneElements_hpp

#include <string>
#include <memory>

#include <Eigen/Eigen>
#include <Eigen/Geometry>

#include "PovraySceneElement.hpp"

///
struct PovrayCameraData {
    Eigen::Vector3f location;
    Eigen::Vector3f up;
    Eigen::Vector3f right;
    Eigen::Vector3f lookAt;
};

/// Returns a vector guaranteed to be perpendicular to `vec`. By convention,
/// if you give it the z-axis, it will return the x-axis.
template <typename T>
typename Eigen::Matrix<T, 3, 1> getPerpindicular(const typename Eigen::Matrix<T, 3, 1> & vec) {
    typedef typename Eigen::Matrix<T, 3, 1> Vec3;
    
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

///
class PovrayCamera : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body);
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const;
    ///
    virtual void write(std::ostream & out) const;
    
    ///
    virtual RayIntersectionResult intersect(const Ray & ray);
    
    ///
    const Eigen::Vector3f & location() const;
    ///
    void setLocation(const Eigen::Vector3f & location);
    
    ///
    const Eigen::Vector3f & up() const;
    ///
    void setUp(const Eigen::Vector3f & up);
    
    ///
    const Eigen::Vector3f & right() const;
    ///
    void setRight(const Eigen::Vector3f & right);
    
    ///
    const Eigen::Vector3f & lookAt() const;
    ///
    void setLookAt(const Eigen::Vector3f & lookAt);
    
    ///
    virtual PovrayPigment const * pigment() const;
    ///
    virtual PovrayFinish const * finish() const;

    ///
    PovrayCameraData data() const {
        PovrayCameraData dat;
        
        dat.location = location_;
        dat.up = up_;
        dat.right = right_;
        dat.lookAt = lookAt_;
        
        return dat;
    }
    
    ///
    Eigen::Vector3f computeOutputEnergyForHit(const RayIntersectionResult & hit, const Eigen::Vector3f & sourceEnergy, bool * hasDiffuse, bool * hasSpecular) {
        return Eigen::Vector3f::Zero();
    }
    
    ///
    Eigen::Vector3f face() const {
        return -right_.cross(up_);
    }
    
    ///
    void orientedTransform(float right, float forward, float up) {
        Eigen::Vector3f diff = right * right_ + forward * face() + up * up_;
        
        location_ += diff;
        lookAt_ += diff;
    }
    
    ///
    void rotate(const Eigen::Vector3f & up, float tilt, float twist) {
        // here we assume "right" never leaves its plane
        
        Eigen::Quaternionf tiltRotation = Eigen::Quaternionf(Eigen::AngleAxisf(tilt, right_));
        Eigen::Quaternionf twistRotation = Eigen::Quaternionf(Eigen::AngleAxisf(twist, up));
        
        Eigen::Matrix3f rotation = (tiltRotation * twistRotation).matrix().block<3,3>(0,0);
        
        Eigen::Vector3f look = (lookAt_ - location_);
        
        lookAt_ = location_ + look.norm() * (rotation * look.normalized());
        up_ = rotation * up_;
        right_ = rotation * right_;
    }

private:

    Eigen::Vector3f location_;
    Eigen::Vector3f up_;
    Eigen::Vector3f right_;
    Eigen::Vector3f lookAt_;
};

///
struct PovrayLightSourceData {
    Eigen::Vector3f position;
    Eigen::Vector4f color;
};

///
class PovrayLightSource : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body);
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const;
    ///
    virtual void write(std::ostream & out) const;

    ///
    virtual RayIntersectionResult intersect(const Ray & ray);

    ///
    virtual PovrayPigment const * pigment() const;
    ///
    virtual PovrayFinish const * finish() const;

    ///
    PovrayLightSourceData data() const {
        PovrayLightSourceData dat;
        
        dat.position = position_;
        dat.color = color_;
        
        return dat;
    }
    
    ///
    Eigen::Vector3f computeOutputEnergyForHit(const RayIntersectionResult & hit, const Eigen::Vector3f & sourceEnergy, bool * hasDiffuse, bool * hasSpecular) {
        return Eigen::Vector3f::Zero();
    }

private:

    Eigen::Vector3f position_;
    Eigen::Vector4f color_;
};

///
struct PovraySphereData {
    Eigen::Vector3f position;
    float radius;
    
    PovrayPigment pigment;
    PovrayFinish finish;
};

///
class PovraySphere : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body);
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const;
    ///
    virtual void write(std::ostream & out) const;

    ///
    virtual RayIntersectionResult intersect(const Ray & ray);
    
    ///
    virtual PovrayPigment const * pigment() const;
    ///
    virtual PovrayFinish const * finish() const;
    
    ///
    PovraySphereData data() const {
        PovraySphereData dat;
        
        dat.position = position_;
        dat.radius = radius_;
        dat.pigment = pigment_;
        dat.finish = finish_;
        
        return dat;
    }
    
    ///
    Eigen::Vector3f computeOutputEnergyForHit(const RayIntersectionResult & hit, const Eigen::Vector3f & sourceEnergy, bool * hasDiffuse, bool * hasSpecular) {
        
        Eigen::Vector3f output;
        Eigen::Vector3f surfacePoint = hit.ray.direction * hit.timeOfIntersection + hit.ray.origin;
        Eigen::Vector3f surfaceNormal = (surfacePoint - position_).normalized();
        /// http://www.cosinekitty.com/raytrace/chapter10_reflection.html
//        Eigen::Vector3f outgoingDirection = hit.ray.direction - 2.0 * (hit.ray.direction.dot(surfaceNormal) * surfaceNormal);
        
        output = (pigment_.color * (finish_.ambient + finish_.diffuse * std::max<float>(0, surfaceNormal.dot(-hit.ray.direction)))).block<3,1>(0,0);
        
        output.x() *= sourceEnergy.x();
        output.y() *= sourceEnergy.y();
        output.z() *= sourceEnergy.z();
        
        if (hasDiffuse != nullptr) {
            *hasDiffuse = true;
        }
        if (hasSpecular != nullptr) {
            *hasSpecular = false;
        }
        
        return output;
    }

private:

    Eigen::Vector3f position_;
    float radius_;
    
    PovrayPigment pigment_;
    PovrayFinish finish_;
    
    Eigen::Vector3f translate_;
};

///
struct PovrayPlaneData {
    Eigen::Vector3f normal;
    float distance;
    
    PovrayPigment pigment;
    PovrayFinish finish;
};

///
class PovrayPlane : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body);
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const;
    ///
    virtual void write(std::ostream & out) const;

    ///
    virtual RayIntersectionResult intersect(const Ray & ray);
    
    ///
    virtual PovrayPigment const * pigment() const;
    ///
    virtual PovrayFinish const * finish() const;

    ///
    PovrayPlaneData data() const {
        PovrayPlaneData dat;
        
        dat.normal = normal_;
        dat.distance = distance_;
        dat.pigment = pigment_;
        dat.finish = finish_;
        
        return dat;
    }
    
    ///
    Eigen::Vector3f computeOutputEnergyForHit(const RayIntersectionResult & hit, const Eigen::Vector3f & sourceEnergy, bool * hasDiffuse, bool * hasSpecular) {
        
        Eigen::Vector3f output;
        Eigen::Vector3f surfaceNormal = normal_;
        /// http://www.cosinekitty.com/raytrace/chapter10_reflection.html
//        Eigen::Vector3f outgoingDirection = hit.ray.direction - 2.0 * (hit.ray.direction.dot(surfaceNormal) * surfaceNormal);
        
        output = (pigment_.color * (finish_.ambient + finish_.diffuse * std::max<float>(0, surfaceNormal.dot(-hit.ray.direction)))).block<3,1>(0,0);
        
        output.x() *= sourceEnergy.x();
        output.y() *= sourceEnergy.y();
        output.z() *= sourceEnergy.z();
        
        if (hasDiffuse != nullptr) {
            *hasDiffuse = true;
        }
        if (hasSpecular != nullptr) {
            *hasSpecular = false;
        }
        
        return output;
    }

private:

    Eigen::Vector3f normal_;
    float distance_;
    
    PovrayPigment pigment_;
    PovrayFinish finish_;
};

#endif /* PovraySceneElements_hpp */
