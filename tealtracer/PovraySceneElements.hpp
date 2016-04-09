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
struct PovrayCameraData {
    Eigen::Vector3f location;
    Eigen::Vector3f up;
    Eigen::Vector3f right;
    Eigen::Vector3f lookAt;
};

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

private:

    Eigen::Vector3f normal_;
    float distance_;
    
    PovrayPigment pigment_;
    PovrayFinish finish_;
};

#endif /* PovraySceneElements_hpp */
