//
//  CPURayTracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "CPURayTracer.hpp"

const Eigen::Vector3f CPURayTracer::Up = Eigen::Vector3f(0.0, 1.0, 0.0);
const Eigen::Vector3f CPURayTracer::Forward = Eigen::Vector3f(0.0, 0.0, -1.0);
const Eigen::Vector3f CPURayTracer::Right = Eigen::Vector3f(1.0, 0.0, 0.0);

Eigen::Matrix4f lookAt(const Eigen::Vector3f & eye, const Eigen::Vector3f & center, const Eigen::Vector3f & up) {
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);
    Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
    
    result(0,0) = s.x();
    result(1,0) = s.y();
    result(2,0) = s.z();
    result(0,1) = u.x();
    result(1,1) = u.y();
    result(2,1) = u.z();
    result(0,2) = -f.x();
    result(1,2) = -f.y();
    result(2,2) = -f.z();
    
    result(3,0) = -s.dot(eye);
    result(3,1) = -u.dot(eye);
    result(3,2) =  f.dot(eye);
    
    return result;
}

///
void CPURayTracer::setupDrawingInWindow(TSWindow * window) {
    ///
    glClearColor(0.3, 0.3, 0.3, 1.0);
    glEnable(GLenum(GL_DEPTH_TEST));
    glDepthFunc(GLenum(GL_LESS));
    
    outputImage.setDimensions(renderOutputWidth, renderOutputHeight);
    target.init(outputImage.width, outputImage.height, outputImage.dataPtr());
    
    jobPool = JobPool(1);
}

void CPURayTracer::start() {
    lastRayTraceTime = glfwGetTime();
    rayTraceElapsedTime = 0.0;
    framesRendered = 0;
    enqueuePhotonMapping();
    enqueRayTrace();
}

void CPURayTracer::enqueRayTrace() {
    jobPool.emplaceJob([=](){
        auto startTime = glfwGetTime();
        this->raytraceScene();
        auto endTime = glfwGetTime();
        lastRayTraceTime = endTime - startTime;
    }, [=](){
        rayTraceElapsedTime = lastRayTraceTime;
        framesRendered++;
        this->target.outputTexture->setNeedsUpdate();
        this->enqueRayTrace();
    });
}

///
void CPURayTracer::drawInWindow(TSWindow * window) {
    glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    target.draw();
    jobPool.checkAndUpdateFinishedJobs();
    
    float FPS = 0;
    if (rayTraceElapsedTime > 0.001) {
        FPS = 1.0 / rayTraceElapsedTime;
    }
    window->setTitle(make_string("CPU Ray Tracer (FPS: ", FPS, ", frames: ", framesRendered, ")"));
}


///
void CPURayTracer::windowResize(TSWindow * window, int w, int h) {

}

///
void CPURayTracer::framebufferResize(TSWindow * window, int w, int h) {
    
}

///
void CPURayTracer::windowClose(TSWindow * window) {

}

///
void CPURayTracer::keyDown(TSWindow * window, int key, int scancode, int mods) {
    
}

///
void CPURayTracer::keyUp(TSWindow * window, int key, int scancode, int mods) {

}

///
void CPURayTracer::mouseUp(TSWindow * window, int button, int mods) {

}

///
void CPURayTracer::mouseDown(TSWindow * window, int button, int mods) {

}

///
void CPURayTracer::mouseMoved(TSWindow * window, double x, double y) {

}

///
void CPURayTracer::mouseScroll(TSWindow * window, double dx, double dy) {

}

///
void CPURayTracer::raytraceScene() {
    assert(scene_ != nullptr);
    
    /// Get the camera
    auto camera = scene_->camera();
    
    /// TODO: build the photon map
    
    /// Create all of the rays
    auto camPos = camera->location();
    auto viewTransform = lookAt(camera->location(), camera->lookAt(), camera->up());
    Eigen::Vector3f forward = (viewTransform * Eigen::Vector4f(Forward.x(), Forward.y(), Forward.z(), 0.0)).block<3,1>(0,0);
    Eigen::Vector3f up = (viewTransform * Eigen::Vector4f(Up.x(), Up.y(), Up.z(), 0.0)).block<3,1>(0,0) * camera->up().norm();
    Eigen::Vector3f right = (viewTransform * Eigen::Vector4f(Right.x(), Right.y(), Right.z(), 0.0)).block<3,1>(0,0) * camera->right().norm();
    
    for (int px = 0; px < outputImage.width; px++) {
        for (int py = 0; py < outputImage.height; py++) {
            Ray ray;
            ray.origin = camPos;
            Eigen::Vector3f pixelPos = camPos + forward - 0.5*up - 0.5*right + right*(0.5+(double)px)/(double)outputImage.width + up*(0.5+(double)py)/(double)outputImage.height;
            ray.direction = (pixelPos - camPos).normalized();
            
            auto hitTest = scene_->closestIntersection(ray);
            Image::Vector4ub color = Image::Vector4ub(0, 0, 0, 255);
            
            if (hitTest.element != nullptr && hitTest.element->pigment() != nullptr) {
                RGBf result = 255.0 * computeOutputEnergyForHit(hitTest);
                for (int i = 0; i < 3; i++) {
                    result(i) = std::min<float>(255.0, result(i));
                }
                
                color.block<3,1>(0,0) = result.cast<uint8_t>();
            }
            
            outputImage.pixel(px, py) = color;
        }
    }
}

RGBf CPURayTracer::computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult) {    
    RGBf output, sourceEnergy;
    Eigen::Vector3f surfaceNormal = hitResult.hit.surfaceNormal;
    const PovrayPigment & pigment = *hitResult.element->pigment();
    const PovrayFinish & finish = *hitResult.element->finish();
    
    sourceEnergy = photonMap.gatherPhotons(100, (int) hitResult.element->id(), hitResult.hit.locationOfIntersection(), surfaceNormal, 150000.0 / (float) photonMap.photons.size());
//    TSLoggerLog(std::cout, "sourceEnergy=", sourceEnergy.norm());
    
    output = (pigment.color * (finish.ambient + finish.diffuse * std::max<float>(0, surfaceNormal.dot(-hitResult.hit.ray.direction)))).block<3,1>(0,0);
    
    output.x() *= sourceEnergy.x();
    output.y() *= sourceEnergy.y();
    output.z() *= sourceEnergy.z();
    
    return output;
}

///
void CPURayTracer::setScene(std::shared_ptr<PovrayScene> scene) {
    scene_ = scene;
}
