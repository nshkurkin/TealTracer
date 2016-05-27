//
//  SCMonteCarloRaytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "SCMonteCarloRaytracer.hpp"

///
void SCMonteCarloRaytracer::raytraceScene() {
    assert(config.scene != nullptr);
    
    /// Get the camera
    auto camera = config.scene->camera();
    auto lights = config.scene->findElements<PovrayLightSource>();
    
    /// TODO: build the photon map
    
    /// Create all of the rays
    auto camPos = camera->location();
    auto viewTransform = lookAt(camera->location(), camera->lookAt(), camera->up());
    Eigen::Vector3f forward = (viewTransform * Eigen::Vector4f(config.Forward.x(), config.Forward.y(), config.Forward.z(), 0.0)).block<3,1>(0,0);
    Eigen::Vector3f up = (viewTransform * Eigen::Vector4f(config.Up.x(), config.Up.y(), config.Up.z(), 0.0)).block<3,1>(0,0) * camera->up().norm();
    Eigen::Vector3f right = (viewTransform * Eigen::Vector4f(config.Right.x(), config.Right.y(), config.Right.z(), 0.0)).block<3,1>(0,0) * camera->right().norm();
    
    for (int px = 0; px < outputImage.width; px++) {
        for (int py = 0; py < outputImage.height; py++) {
            Ray ray;
            ray.origin = camPos;
            ray.direction = (forward - 0.5*up - 0.5*right + right*(0.5+(double)px)/(double)outputImage.width + up*(0.5+(double)py)/(double)outputImage.height).normalized();
            
            auto hitTest = config.scene->closestIntersection(ray);
            Image<uint8_t>::Vector4 color = Image<uint8_t>::Vector4(0, 0, 0, 255);
            
            if (hitTest.element != nullptr && hitTest.element->pigment() != nullptr) {
                /// Get indirect lighting
                RGBf result = RGBf(0,0,0);
                
                /// Get direct lighting
                for (auto lightItr = lights.begin(); lightItr != lights.end(); lightItr++) {
                    auto light = *lightItr;
                    Eigen::Vector3f hitLoc = hitTest.hit.locationOfIntersection();
                    Eigen::Vector3f toLight = light->position() - hitLoc;
                    Eigen::Vector3f toLightDir = toLight.normalized();
                    Ray shadowRay;
                    shadowRay.origin = hitLoc + 0.01f * toLightDir;
                    shadowRay.direction = toLightDir;
                    auto shadowHitTest = config.scene->closestIntersection(shadowRay);
                    
                    bool isShadowed = !(!shadowHitTest.hit.intersected
                     || (shadowHitTest.hit.intersected && shadowHitTest.hit.timeOfIntersection > toLight.norm()));
                    
                    if (!isShadowed) {
                        result += 255.0 * computeOutputEnergyForHit(hitTest, toLightDir, (camPos - hitLoc).normalized(), light->color().block<3,1>(0,0));
                    }
                }
                
                for (int i = 0; i < 3; i++) {
                    result(i) = std::min<float>(255.0, result(i));
                }
                
                color.block<3,1>(0,0) = result.cast<uint8_t>();
            }
            
            outputImage.pixel(px, py) = color;
        }
    }
}
