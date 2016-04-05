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
        /// https://en.wikipedia.org/wiki/Vector_projection
        Eigen::Vector3f toCenter = position_ - ray.origin;
        Eigen::Vector3f alongRayDir = toCenter.dot(ray.direction) * ray.direction;
        Eigen::Vector3f distVec = toCenter - alongRayDir;
        float sqrDist = distVec.dot(distVec);
        
        result.intersected = sqrDist <= radius_ * radius_;
        if (result.intersected) {
            result.timeOfIntersection = alongRayDir.norm();
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
        if (product > 0.001 && product < -0.001) {
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
