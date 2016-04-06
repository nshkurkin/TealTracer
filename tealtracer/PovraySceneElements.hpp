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

#include "PovraySceneElement.hpp"

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
    virtual RayIntersectionResult intersect(const Ray & ray) {
        RayIntersectionResult result;
        result.intersected = false;
        return result;
    }
    
    ///
    const Eigen::Vector3f & location() const {
        return location_;
    }
    
    ///
    void setLocation(const Eigen::Vector3f & location) {
        location_ = location;
    }
    
    ///
    const Eigen::Vector3f & up() const {
        return up_;
    }
    
    ///
    void setUp(const Eigen::Vector3f & up) {
        up_ = up;
    }
    
    ///
    const Eigen::Vector3f & right() const {
        return right_;
    }
    
    ///
    void setRight(const Eigen::Vector3f & right) {
        right_ = right;
    }
    
    ///
    const Eigen::Vector3f & lookAt() const {
        return lookAt_;
    }
    
    ///
    void setLookAt(const Eigen::Vector3f & lookAt) {
        lookAt_ = lookAt;
    }
    

private:

    Eigen::Vector3f location_;
    Eigen::Vector3f up_;
    Eigen::Vector3f right_;
    Eigen::Vector3f lookAt_;
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
    virtual RayIntersectionResult intersect(const Ray & ray) {
        RayIntersectionResult result;
        result.intersected = false;
        return result;
    }

private:

    Eigen::Vector3f position_;
    Eigen::Vector4f color_;
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
    virtual RayIntersectionResult intersect(const Ray & ray) {
        RayIntersectionResult result;
        
        float A = ray.direction.dot(ray.direction);
        float B = 2.0 * (ray.origin - position_).dot(ray.direction);
        float C = (ray.origin - position_).dot(ray.origin - position_) - radius_ * radius_;
        
        float radical = B*B - 4.0*A*C;
        if (radical >= 0) {
            float sqrRadical = std::sqrt(radical);
            float t0 = (-B + sqrRadical)/(2.0 * A);
            float t1 = (-B - sqrRadical)/(2.0 * A);
            result.intersected = t0 >= 0 || t1 >= 0;
            if (t0 >= 0 && t1 >= 0) {
                result.timeOfIntersection = std::min(t0, t1);
            }
            else if (t0 >= 0) {
                result.timeOfIntersection = t0;
            }
            else if (t1 >= 0) {
                result.timeOfIntersection = t1;
            }
        }
        
        return result;
    }

private:

    Eigen::Vector3f position_;
    float radius_;
    
    PovrayPigment pigment_;
    PovrayFinish finish_;
    
    Eigen::Vector3f translate_;
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
    virtual RayIntersectionResult intersect(const Ray & ray) {
        RayIntersectionResult result;
        /// https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
        float product = ray.direction.dot(normal_);
        if (product > 0.001 || product < -0.001) {
            result.timeOfIntersection = -(ray.origin.dot(normal_) - distance_) / product;
        }
        
        result.intersected = result.timeOfIntersection >= 0.0;
        return result;
    }

private:

    Eigen::Vector3f normal_;
    float distance_;
    
    PovrayPigment pigment_;
    PovrayFinish finish_;
};

#endif /* PovraySceneElements_hpp */
