//
//  PovraySceneElement.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PovraySceneElement_hpp
#define PovraySceneElement_hpp

#include <string>
#include <memory>
#include <vector>
#include <map>

#include <Eigen/Eigen>

#include "TSLogger.hpp"

///
struct Ray {
    Eigen::Vector3f origin;
    Eigen::Vector3f direction;
};

///
struct RayIntersectionResult {
    bool intersected;
    float timeOfIntersection;
    Ray ray;
    Eigen::Vector3f surfaceNormal;
    
    ///
    RayIntersectionResult() : intersected(false), timeOfIntersection(0) {}
    
    ///
    Eigen::Vector3f locationOfIntersection() const {
        return ray.origin + (ray.direction * timeOfIntersection);
    }
    
    ///
    Eigen::Vector3f outgoingDirection() const {
        /// http://www.cosinekitty.com/raytrace/chapter10_reflection.html
        return ray.direction - 2.0 * (ray.direction.dot(surfaceNormal) * surfaceNormal);
    }
};

///
struct PovrayPigment {
    Eigen::Vector4f color;
    
    ///
    PovrayPigment() : color(0, 0, 0, 1) {}
};

///
struct PovrayFinish {
    float ambient;
    float diffuse;
    float specular;
    float roughness;
    
    ///
    PovrayFinish() : ambient(0), diffuse(0), specular(0), roughness(0) {}
};

///


///
class PovraySceneElement {
public:
    virtual ~PovraySceneElement();
    /// Sets this element's content to "body"
    virtual void parse(const std::string & body) = 0;
    ///
    virtual std::shared_ptr<PovraySceneElement> copy() const = 0;
    ///
    virtual void write(std::ostream & out) const {}
    
    ///
    virtual RayIntersectionResult intersect(const Ray & ray) = 0;
    
    ///
    virtual PovrayPigment const * pigment() const = 0;
    ///
    virtual PovrayFinish const * finish() const = 0;
    
    uint16_t id() const {return id_;}
    
protected:

    friend class PovrayScene;
    uint16_t id_;

    ///
    enum ValueType {
        Float,
        Vec3,
        Vec4,
        Pigment,
        Finish
    };

    ///
    void parseBody(std::string & body, const std::map<std::string, std::pair<ValueType, void *>> & contentMapping);
    /// Body is of the form "( )<#, #, # >( ...)"
    bool parseVector3(std::string & body, Eigen::Vector3f & vec);
    ///
    bool parseVector4(std::string & body, Eigen::Vector4f & vec);
    /// Body is of the form "( )#( ...)"
    bool parseFloat(std::string & body, float & val);
    ///
    bool parsePigment(std::string & body, PovrayPigment & pigment);
    ///
    bool parseFinish(std::string & body, PovrayFinish & finish);
    
    ///
    static inline char writeOut(std::ostream & out, const Eigen::Vector3f & vec) {
        out << "<" << vec[0] << ", " << vec[1] << ", " << vec[2] << ">";
        return ' ';
    }
    ///
    static inline char writeOut(std::ostream & out, const Eigen::Vector4f & vec) {
        out << "<" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << ">";
        return ' ';
    }
    ///
    static inline char writeOut(std::ostream & out, float val) {
        out << val;
        return ' ';
    }
    ///
    static inline char writeOut(std::ostream & out, const PovrayPigment & pigment) {
        out << "pigment { color rgbf " << writeOut(out, pigment.color) << "} ";
        return ' ';
    }
    ///
    static inline char writeOut(std::ostream & out, const PovrayFinish & finish) {
        out << "finish { ambient " << writeOut(out, finish.ambient) << " diffuse " << writeOut(out, finish.diffuse) << " specular " << writeOut(out, finish.specular) << " roughness " << writeOut(out, finish.roughness) << "} ";
        return ' ';
    }
};

#endif /* PovraySceneElement_hpp */
