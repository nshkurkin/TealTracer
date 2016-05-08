//
//  PovraySceneElements.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PovraySceneElements.hpp"

/// Sets this element's content to "body"
void PovrayCamera::parse(const std::string & body) {
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
RayIntersectionResult PovrayCamera::intersect(const Ray & ray) {
    RayIntersectionResult result;
    result.intersected = false;
    return result;
}

///
const Eigen::Vector3f & PovrayCamera::location() const {
    return location_;
}

///
void PovrayCamera::setLocation(const Eigen::Vector3f & location) {
    location_ = location;
}

///
const Eigen::Vector3f & PovrayCamera::up() const {
    return up_;
}

///
void PovrayCamera::setUp(const Eigen::Vector3f & up) {
    up_ = up;
}

///
const Eigen::Vector3f & PovrayCamera::right() const {
    return right_;
}

///
void PovrayCamera::setRight(const Eigen::Vector3f & right) {
    right_ = right;
}

///
const Eigen::Vector3f & PovrayCamera::lookAt() const {
    return lookAt_;
}

///
void PovrayCamera::setLookAt(const Eigen::Vector3f & lookAt) {
    lookAt_ = lookAt;
}

///
PovrayPigment const * PovrayCamera::pigment() const {
    return nullptr;
}

///
PovrayFinish const * PovrayCamera::finish() const {
    return nullptr;
}

///
std::shared_ptr<PovraySceneElement> PovrayCamera::copy() const {
    auto camera = std::shared_ptr<PovrayCamera>(new PovrayCamera());
    camera->location_ = location_;
    camera->up_ = up_;
    camera->right_ = right_;
    camera->lookAt_ = lookAt_;
    return camera;
}

///
void PovrayCamera::write(std::ostream & out) const {
    out << "camera {" << std::endl;
    out << "\tlocation\t" << writeOut(out, location_) << std::endl;
    out << "\tup\t\t" << writeOut(out, up_) << std::endl;
    out << "\tright\t" << writeOut(out, right_) << std::endl;
    out << "\tlook_at\t" << writeOut(out, lookAt_) << std::endl;
    out << "}" << std::endl;
}

/// Sets this element's content to "body"
void PovrayLightSource::parse(const std::string & body) {
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
std::shared_ptr<PovraySceneElement> PovrayLightSource::copy() const {
    auto source = std::shared_ptr<PovrayLightSource>(new PovrayLightSource());
    source->position_ = position_;
    source->color_ = color_;
    return source;
}

///
void PovrayLightSource::write(std::ostream & out) const {
    out << "light_source {" << writeOut(out, position_) << " color rgbf " << writeOut(out, color_) << "}" << std::endl;
}

///
RayIntersectionResult PovrayLightSource::intersect(const Ray & ray) {
    RayIntersectionResult result;
    result.intersected = false;
    return result;
}

///
PovrayPigment const * PovrayLightSource::pigment() const {
    return nullptr;
}

///
PovrayFinish const * PovrayLightSource::finish() const {
    return nullptr;
}

/// Sets this element's content to "body"
void PovraySphere::parse(const std::string & body) {
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
std::shared_ptr<PovraySceneElement> PovraySphere::copy() const {
    auto sphere = std::shared_ptr<PovraySphere>(new PovraySphere());
    sphere->position_ = position_;
    sphere->radius_ = radius_;
   
    sphere->pigment_ = pigment_;
    sphere->finish_ = finish_;

    sphere->translate_ = translate_;
    return sphere;
}

///
void PovraySphere::write(std::ostream & out) const {
    out << "sphere {" << writeOut(out, position_) <<  ", " << writeOut(out, radius_) << std::endl;
    out << "\tpigment\t" << writeOut(out, pigment_) << std::endl;
    out << "\tfinish\t" << writeOut(out, finish_) << std::endl;
    out << "\ttranslate\t" << writeOut(out, translate_) << std::endl;
    out << "}" << std::endl;
}

///
RayIntersectionResult PovraySphere::intersect(const Ray & ray) {
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
        result.ray = ray;
        if (t0 >= 0 && t1 >= 0) {
            result.timeOfIntersection = std::min(t0, t1);
        }
        else if (t0 >= 0) {
            result.timeOfIntersection = t0;
        }
        else if (t1 >= 0) {
            result.timeOfIntersection = t1;
        }
        
        if (result.timeOfIntersection > 0) {
            result.surfaceNormal = (result.locationOfIntersection() - position_).normalized();
        }
    }
    
    return result;
}

///
PovrayPigment const * PovraySphere::pigment() const {
    return &pigment_;
}

///
PovrayFinish const * PovraySphere::finish() const {
    return &finish_;
}

/// Sets this element's content to "body"
void PovrayPlane::parse(const std::string & body) {
    std::map<std::string, std::pair<ValueType, void *>> content;
//        TSLoggerLog(std::cout, "parsing plane");
    
    content["pigment"] = std::make_pair(PovraySceneElement::ValueType::Pigment, &pigment_);
    content["finish"] = std::make_pair(PovraySceneElement::ValueType::Finish, &finish_);
    
    std::string localBody = body;
    
    parseVector3(localBody, normal_);
    auto commaLoc = localBody.find(",");
    localBody = localBody.substr(commaLoc + 1, localBody.length() - commaLoc - 1);
    parseFloat(localBody, distance_);
    parseBody(localBody, content);
}

///
std::shared_ptr<PovraySceneElement> PovrayPlane::copy() const {
    auto plane = std::shared_ptr<PovrayPlane>(new PovrayPlane());
    plane->normal_ = normal_;
    plane->distance_ = distance_;

    plane->pigment_ = pigment_;
    plane->finish_ = finish_;
    
    return plane;
}

///
void PovrayPlane::write(std::ostream & out) const {
    out << "plane {" << writeOut(out, normal_) <<  ", " << writeOut(out, distance_) << std::endl;
    out << "\tpigment\t" << writeOut(out, pigment_) << std::endl;
    out << "\tfinish\t" << writeOut(out, finish_) << std::endl;
    out << "}" << std::endl;
}

///
RayIntersectionResult PovrayPlane::intersect(const Ray & ray) {
    RayIntersectionResult result;
    /// https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
    float product = ray.direction.dot(normal_);
    if (product > 0.001 || product < -0.001) {
        result.timeOfIntersection = -(ray.origin.dot(normal_) - distance_) / product;
        result.ray = ray;
        result.surfaceNormal = normal_;
    }
    
    result.intersected = result.timeOfIntersection > 0.0;
    return result;
}

///
PovrayPigment const * PovrayPlane::pigment() const {
    return &pigment_;
}

///
PovrayFinish const * PovrayPlane::finish() const {
    return &finish_;
}

