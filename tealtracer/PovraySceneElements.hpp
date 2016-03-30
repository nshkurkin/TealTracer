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

#include "PovrayScene.hpp"

///
class PovrayCamera : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) {
        
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto camera = std::shared_ptr<PovrayCamera>(new PovrayCamera());
        camera->location_ = location_;
        camera->up_ = up_;
        camera->right_ = right_;
        camera->lookAt_ = lookAt_;
        return camera;
    }

private:

    Eigen::Matrix3f location_;
    Eigen::Matrix3f up_;
    Eigen::Matrix3f right_;
    Eigen::Matrix3f lookAt_;
};

///
class PovrayLightSource : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) {
        
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto source = std::shared_ptr<PovrayLightSource>(new PovrayLightSource());
        source->position_ = position_;
        source->color_ = color_;
        return source;
    }

private:

    Eigen::Matrix3f position_;
    Eigen::Matrix4f color_;
};


///
class PovraySphere : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) {
        
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto sphere = std::shared_ptr<PovraySphere>(new PovraySphere());
        sphere->position_ = position_;
        sphere->radius_ = radius_;
        return sphere;
    }

private:

    Eigen::Matrix3f position_;
    float radius_;
    // pigment
    // finish
    // translate
};

///
class PovrayPlane : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) {
        
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto plane = std::shared_ptr<PovrayPlane>(new PovrayPlane());
        plane->position_ = position_;
        return plane;
    }

private:

    Eigen::Matrix3f position_;
    // ??? z-position?
    // pigment
    // finish
};

#endif /* PovraySceneElements_hpp */
