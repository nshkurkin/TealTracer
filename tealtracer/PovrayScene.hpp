//
//  PovrayScene.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef PovrayScene_hpp
#define PovrayScene_hpp

#include <string>
#include <memory>
#include <vector>
#include <map>

#include <Eigen/Eigen>

#include "TSLogger.hpp"
#include "PovraySceneElement.hpp"
#include "PovraySceneElements.hpp"

///
class PovrayScene {
public:

    ///
    void addElement(std::shared_ptr<PovraySceneElement> element);
    ///
    static std::shared_ptr<PovrayScene> loadScene(const std::string & file);

    ///
    void writeOut(std::ostream & out) {
        for (auto itr = elements_.begin(); itr != elements_.end(); itr++) {
            (*itr)->write(out);
        }
    }
    
    ///
    template <class C>
    std::vector<std::shared_ptr<C>> findElements() const {
        std::vector<std::shared_ptr<C>> elements;
        for (auto itr = elements_.begin(); itr != elements_.end(); itr++) {
            auto ptr = *itr;
            auto cast = std::dynamic_pointer_cast<C>(ptr);
            if (cast != nullptr) {
                elements.push_back(cast);
            }
        }
        return elements;
    }
    
    ///
    std::shared_ptr<PovrayCamera> camera() {
    
        if (cachedCamera == nullptr) {
            cachedCamera = findElements<PovrayCamera>()[0];
        }
    
        return cachedCamera;
    }
    
private:

    std::shared_ptr<PovrayCamera> cachedCamera;

public:
    
    struct InstersectionResult {
        std::shared_ptr<PovraySceneElement> element;
        RayIntersectionResult hit;
    };
    
    ///
    InstersectionResult closestIntersection(const Ray & ray) {
        InstersectionResult result;
        result.element = nullptr;
        result.hit.timeOfIntersection = std::numeric_limits<float>::infinity();
        result.hit.ray = ray;
        
        for (auto itr = elements_.begin(); itr != elements_.end(); itr++) {
            auto hitTest = (*itr)->intersect(ray);
            if (hitTest.intersected && hitTest.timeOfIntersection < result.hit.timeOfIntersection) {
                result.element = *itr;
                result.hit = hitTest;
//                result.hit.timeOfIntersection = hitTest.timeOfIntersection;
            }
        }
        
        return result;
    }
    
    ///
    std::vector<InstersectionResult> intersections(const Ray & ray) {
        std::vector<InstersectionResult> results;
        
        for (auto itr = elements_.begin(); itr != elements_.end(); itr++) {
            auto hitTest = (*itr)->intersect(ray);
            if (hitTest.intersected) {
                InstersectionResult result;
            
                result.element = *itr;
                result.hit = hitTest;
                
                int newIndex = (int) results.size();
                results.push_back(result);
                if (results[newIndex].hit.timeOfIntersection < results[0].hit.timeOfIntersection) {
                    results[newIndex] = results[0];
                    results[0] = result;
                }
            }
        }
        
        return results;
    }
    
    ///
    std::shared_ptr<PovraySceneElement> elementForId(uint16_t id) {
        return elements_[(int) id];
    }

private:

    ///
    std::vector<std::shared_ptr<PovraySceneElement>> elements_;
};

#endif /* PovrayScene_hpp */
