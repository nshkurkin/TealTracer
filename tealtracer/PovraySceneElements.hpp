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
    virtual void parse(const std::string & body) {
//        TSLoggerLog(std::cout, "parsing camera");
        std::map<std::string, std::pair<ValueType, void *>> content;
        
        content["location"] = std::make_pair(PovraySceneElement::ValueType::Vec3, &location_);
        content["up"] = std::make_pair(PovraySceneElement::ValueType::Vec3, &up_);
        content["right"] = std::make_pair(PovraySceneElement::ValueType::Vec3, &right_);
        content["look_at"] = std::make_pair(PovraySceneElement::ValueType::Vec3, &lookAt_);
        
        std::string localBody = body;
        parseBody(localBody, content);
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

    ///
    virtual void write(std::ostream & out) const {
        out << "camera {" << std::endl;
        out << "\tlocation\t" << writeOut(out, location_) << std::endl;
        out << "\tup\t\t" << writeOut(out, up_) << std::endl;
        out << "\tright\t" << writeOut(out, right_) << std::endl;
        out << "\tlook_at\t" << writeOut(out, lookAt_) << std::endl;
        out << "}" << std::endl;
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
    virtual void parse(const std::string & body) {
//        TSLoggerLog(std::cout, "parsing light source");
        color_ << 0, 0, 0, 1;
        std::map<std::string, std::pair<ValueType, void *>> content;
        
        content["color rgb"] = std::make_pair(PovraySceneElement::ValueType::Vec3, &color_);
        content["color rgbf"] = std::make_pair(PovraySceneElement::ValueType::Vec4, &color_);
        
        std::string localBody = body;
        
        parseVector3(localBody, position_);
        parseBody(localBody, content);
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto source = std::shared_ptr<PovrayLightSource>(new PovrayLightSource());
        source->position_ = position_;
        source->color_ = color_;
        return source;
    }
    
    ///
    virtual void write(std::ostream & out) const {
        out << "light_source {" << writeOut(out, position_) << " color rgbf " << writeOut(out, color_) << "}" << std::endl;
    }

private:

    Eigen::Vector3f position_;
    Eigen::Vector4f color_;
};


///
class PovraySphere : public PovraySceneElement {
public:

    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) {
//        TSLoggerLog(std::cout, "parsing sphere");
        std::map<std::string, std::pair<ValueType, void *>> content;
        
        content["pigment"] = std::make_pair(PovraySceneElement::ValueType::Pigment, &pigment_);
        content["finish"] = std::make_pair(PovraySceneElement::ValueType::Finish, &finish_);
        content["translate"] = std::make_pair(PovraySceneElement::ValueType::Vec3, &translate_);
        
        std::string localBody = body;
        
        parseVector3(localBody, position_);
        auto commaLoc = localBody.find(",");
        localBody = localBody.substr(commaLoc + 1, localBody.length() - commaLoc - 1);
        parseFloat(localBody, radius_);
        parseBody(localBody, content);
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto sphere = std::shared_ptr<PovraySphere>(new PovraySphere());
        sphere->position_ = position_;
        sphere->radius_ = radius_;
       
        sphere->pigment_ = pigment_;
        sphere->finish_ = finish_;
    
        sphere->translate_ = translate_;
        return sphere;
    }
    
    ///
    virtual void write(std::ostream & out) const {
        out << "sphere {" << writeOut(out, position_) <<  ", " << writeOut(out, radius_) << std::endl;
        out << "\tpigment\t" << writeOut(out, pigment_) << std::endl;
        out << "\tfinish\t" << writeOut(out, finish_) << std::endl;
        out << "\ttranslate\t" << writeOut(out, translate_) << std::endl;
        out << "}" << std::endl;
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
    virtual void parse(const std::string & body) {
        std::map<std::string, std::pair<ValueType, void *>> content;
//        TSLoggerLog(std::cout, "parsing plane");
        
        content["pigment"] = std::make_pair(PovraySceneElement::ValueType::Pigment, &pigment_);
        content["finish"] = std::make_pair(PovraySceneElement::ValueType::Finish, &finish_);
        
        std::string localBody = body;
        
        parseVector3(localBody, position_);
        auto commaLoc = localBody.find(",");
        localBody = localBody.substr(commaLoc + 1, localBody.length() - commaLoc - 1);
        parseFloat(localBody, depth_);
        parseBody(localBody, content);
    }
    
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const {
        auto plane = std::shared_ptr<PovrayPlane>(new PovrayPlane());
        plane->position_ = position_;
        plane->depth_ = depth_;
    
        plane->pigment_ = pigment_;
        plane->finish_ = finish_;
        
        return plane;
    }
    
    ///
    virtual void write(std::ostream & out) const {
        out << "plane {" << writeOut(out, position_) <<  ", " << writeOut(out, depth_) << std::endl;
        out << "\tpigment\t" << writeOut(out, pigment_) << std::endl;
        out << "\tfinish\t" << writeOut(out, finish_) << std::endl;
        out << "}" << std::endl;
    }

private:

    Eigen::Vector3f position_;
    float depth_;
    
    PovrayPigment pigment_;
    PovrayFinish finish_;
};

#endif /* PovraySceneElements_hpp */
