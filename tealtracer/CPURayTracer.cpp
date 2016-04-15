//
//  CPURayTracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "CPURayTracer.hpp"

#include "PhotonKDTree.hpp"

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
    
    photonMapType = KDTree;
    photonBounceProbability = 0.5;
    photonBounceEnergyMultipler = 0.5;
    
    jobPool = JobPool(1);
    distribution = std::uniform_real_distribution<float>(0.0,1.0);
}

void CPURayTracer::start() {

    switch (photonMapType) {
    case KDTree: {
        auto * map = new PhotonKDTree();
        photonMap = std::shared_ptr<PhotonMap>(map);
        break;
    }
    case Hashmap: {
        auto * map = new PhotonHashmap();
        map->setDimensions(Eigen::Vector3f(-20,-20,-20), Eigen::Vector3f(20,20,20));
        photonMap = std::shared_ptr<PhotonMap>(map);
        break;
    }
    default:
        assert(false);
        break;
    }
    
    switch (brdfType) {
    case BlinnPhong: {
        brdf = std::shared_ptr<BRDF>(new BlinnPhongBRDF());
        break;
    }
    case OrenNayar: {
        brdf = std::shared_ptr<BRDF>(new OrenNayarBRDF());
        break;
    }
    default:
        assert(false);
        break;
    }

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
                RGBf result = 255.0 * computeOutputEnergyForHit(hitTest, Eigen::Vector3f::Zero(), -ray.direction, true);
                for (int i = 0; i < 3; i++) {
                    result(i) = std::min<float>(255.0, result(i));
                }
                
                color.block<3,1>(0,0) = result.cast<uint8_t>();
            }
            
            outputImage.pixel(px, py) = color;
        }
    }
}

RGBf CPURayTracer::computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, bool usePhotonMap) {
    
    RGBf output = RGBf::Zero();
    brdf->pigment = *hitResult.element->pigment();
    brdf->finish = *hitResult.element->finish();
    
    if (usePhotonMap) {
        auto photonInfo = photonMap->gatherPhotonsIndices(numberOfPhotonsToGather, std::numeric_limits<float>::infinity(), hitResult.hit.locationOfIntersection());
        
        float maxSqrDist = 0.001;
        //  Accumulate radiance of the K nearest photons
        for (int i = 0; i < photonInfo.size(); ++i) {
            
            const auto & p = photonMap->photons[photonInfo[i].index];
            
            RGBf photonEnergy = RGBf::Zero(), surfaceEnergy = RGBf::Zero();
            if (photonInfo[i].squareDistance > maxSqrDist) {
                maxSqrDist = photonInfo[i].squareDistance;
            }
            
            photonEnergy = brdf->computeColor(rgbe2rgb(p.energy), -p.incomingDirection.vector(), toViewer, hitResult.hit.surfaceNormal);
            surfaceEnergy = brdf->computeColor(RGBf(1,1,1), -p.incomingDirection.vector(), toViewer, hitResult.hit.surfaceNormal);
            
            output.x() += photonEnergy.x() * surfaceEnergy.x();
            output.y() += photonEnergy.y() * surfaceEnergy.y();
            output.z() += photonEnergy.z() * surfaceEnergy.z();
        }
        
        output = output / (M_PI * maxSqrDist);
        
    }
    else {
       output = brdf->computeColor(RGBf(1,1,1), toLight, toViewer, hitResult.hit.surfaceNormal);
    }
    
    return output;
}

///
void CPURayTracer::setScene(std::shared_ptr<PovrayScene> scene) {
    scene_ = scene;
}
