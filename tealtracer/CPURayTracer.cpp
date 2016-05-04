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

CPURayTracer::CPURayTracer() {
    photonMapType = KDTree;
    photonBounceProbability = 0.5;
    photonBounceEnergyMultipler = 0.5;
    
    hashmapCellsize = 2.0;
    hashmapSpacing = 2;
    hashmapGridStart = Eigen::Vector3f(-20, -20, -20);
    hashmapGridEnd = Eigen::Vector3f(20, 20, 20);
    
    jobPool = JobPool(1);
    distribution = std::uniform_real_distribution<float>(0.0,1.0);
}

///
void CPURayTracer::setupDrawingInWindow(TSWindow * window) {
    ///
    glClearColor(0.3, 0.3, 0.3, 1.0);
    glEnable(GLenum(GL_DEPTH_TEST));
    glDepthFunc(GLenum(GL_LESS));
    
    outputImage.setDimensions(renderOutputWidth, renderOutputHeight);
    target.init(outputImage.width, outputImage.height, outputImage.dataPtr());
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
        map->cellsize = hashmapCellsize;
        map->spacing = hashmapSpacing;
        map->setDimensions(hashmapGridStart, hashmapGridEnd);
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
///
void CPURayTracer::enqueuePhotonMapping() {
    jobPool.emplaceJob([=]() {
        TSLoggerLog(std::cout, "[", glfwGetTime(), "] Started building photon map");
        buildPhotonMap();
    }, [=]() {
        TSLoggerLog(std::cout, "[", glfwGetTime(), "] Finished building photon map");
    });
}

///
void CPURayTracer::buildPhotonMap() {
    assert(photonMap != nullptr);
    photonMap->photons.clear();
    emitPhotons();
    photonMap->buildMap();
}

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution;

///
void CPURayTracer::emitPhotons() {
    /// for each light, emit photons into the scene.
    auto lights = scene_->findElements<PovrayLightSource>();
    float lumens = lumensPerLight;
    int numRays = raysPerLight;
    float luminosityPerPhoton = lumens/(float)numRays;
    
    for (int photonItr = 0; photonItr < numRays; photonItr++) {
        bool photonStored = false;
        while (!photonStored) {
            auto light = lights[(int) (distribution(generator) * (float) lights.size()) % lights.size()];
            
            float u = distribution(generator);
            float v = distribution(generator);
            
            Ray ray;
            
            ray.origin = light->position();
            ray.direction = light->getSampleDirection(u, v);
            
            processEmittedPhoton(light->color().block<3,1>(0,0) * luminosityPerPhoton, ray, &photonStored);
        }
    }
    
//    
//    for (auto itr = lights.begin(); itr != lights.end(); itr++) {
//        auto light = *itr;
//        auto color = light->color();
//
//        float lumens = lumensPerLight;
//        int numRays = raysPerLight;
//        float luminosityPerPhoton = lumens/(float)numRays;
//
//        for (int i = 0; i < numRays; i++) {
//            float u = distribution(generator);
//            float v = distribution(generator);
//            
//            Ray ray;
//            
//            ray.origin = light->position();
//            ray.direction = light->getSampleDirection(u, v);
//            
//            auto hits = scene_->intersections(ray);
//            processHits(color.block<3,1>(0,0) * luminosityPerPhoton, ray, hits);
//        }
//    }
    
    TSLoggerLog(std::cout, "Photons=", photonMap->photons.size());
}

void CPURayTracer::processEmittedPhoton(
    ///
    RGBf sourceLightEnergy,
    const Ray & initialRay,
    
    ///
    bool * photonStored
    ) {

    *photonStored = false;
    
    Ray ray;
    ray.origin = initialRay.origin;
    ray.direction = initialRay.direction;
    RGBf energy = sourceLightEnergy;
    
    auto hits = scene_->intersections(ray);

    while (!*photonStored && hits.size() > 0) {
        struct JensenPhoton photon;
        auto hit = hits[0];
        
        photon.position = hit.hit.locationOfIntersection();
        photon.incomingDirection = CompressedNormalVector3(ray.direction);
        photon.energy = rgb2rgbe(energy);
        photon.flags.geometryIndex = hit.element->id();

        if (mapShadowPhotons) {
            for (int i = 1; i < hits.size(); i++) {
                const auto & hitResult = hits[i];
                photonMap->photons.push_back(JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, RGBf::Zero(), true, false, hitResult.element->id()));
            }
        }

        float value = distribution(generator);

        if (value < photonBounceProbability) {

            Ray reflectedRay;
            reflectedRay.direction = hit.hit.outgoingDirection();
            reflectedRay.origin = hit.hit.locationOfIntersection() + 0.001f * reflectedRay.direction;
            
            RGBf hitEnergy = computeOutputEnergyForHit(hit, -ray.direction, hit.hit.outgoingDirection(), energy, false) * photonBounceEnergyMultipler;
            //////
            
            /// Calculate intersection
            hits = scene_->intersections(reflectedRay);
            ray.origin = reflectedRay.origin;
            ray.direction = reflectedRay.direction;
            energy = hitEnergy;
        }
        else {
            static const float factor = 10000.0f;
            photon.position(0) =  std::floor(photon.position.x() * factor) / factor;
            photon.position(1) =  std::floor(photon.position.y() * factor) / factor;
            photon.position(2) =  std::floor(photon.position.z() * factor) / factor;
            photonMap->photons.push_back(photon);
            *photonStored = true;
        }
    }
}

///
void CPURayTracer::processHits(const RGBf & energy, const Ray & ray, const std::vector<PovrayScene::InstersectionResult> & hits) {
    /// Add in shadow photons
    for (int i = 1; i < hits.size(); i++) {
        const auto & hitResult = hits[i];
        photonMap->photons.push_back(JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, RGBf::Zero(), true, false, hitResult.element->id()));
    }

    /// bounce around the other photon
    if (hits.size() > 0) {
        const auto & hitResult = hits[0];
        JensenPhoton photon = JensenPhoton(hitResult.hit.locationOfIntersection(), hitResult.hit.ray.direction, energy, false, false, hitResult.element->id());
        
        if (distribution(generator) < photonBounceProbability) {
            Ray reflectedRay;
            
            reflectedRay.direction = hitResult.hit.outgoingDirection();
            reflectedRay.origin = hitResult.hit.locationOfIntersection() + 0.001 * reflectedRay.direction;
            
            auto hitEnergy = computeOutputEnergyForHit(hitResult, -ray.direction, hitResult.hit.outgoingDirection(), energy, false) * photonBounceEnergyMultipler;
            auto newHits = scene_->intersections(reflectedRay);
            
            processHits(hitEnergy, reflectedRay, newHits);
        }
        else {
            photonMap->photons.push_back(photon);
        }
    }
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
            Image<uint8_t>::Vector4 color = Image<uint8_t>::Vector4(0, 0, 0, 255);
            
            if (hitTest.element != nullptr && hitTest.element->pigment() != nullptr) {
                RGBf result = 255.0 * computeOutputEnergyForHit(hitTest, Eigen::Vector3f::Zero(), -ray.direction, RGBf(1,1,1), true);
                for (int i = 0; i < 3; i++) {
                    result(i) = std::min<float>(255.0, result(i));
                }
                
                color.block<3,1>(0,0) = result.cast<uint8_t>();
            }
            
            outputImage.pixel(px, py) = color;
        }
    }
}

RGBf CPURayTracer::computeOutputEnergyForHit(const PovrayScene::InstersectionResult & hitResult, const Eigen::Vector3f & toLight, const Eigen::Vector3f & toViewer, const RGBf & sourceEnergy, bool usePhotonMap) {
    
    RGBf output = RGBf::Zero();
    brdf->pigment = *hitResult.element->pigment();
    brdf->finish = *hitResult.element->finish();
    
    if (usePhotonMap) {
        auto photonInfo = photonMap->gatherPhotonsIndices(numberOfPhotonsToGather, std::numeric_limits<float>::infinity(), hitResult.hit.locationOfIntersection());
        
        float maxSqrDist = 0.001;
        //  Accumulate radiance of the K nearest photons
        for (int i = 0; i < photonInfo.size(); ++i) {
            
            const auto & p = photonMap->photons[photonInfo[i].index];
            
            RGBf photonEnergy = RGBf::Zero();
            if (photonInfo[i].squareDistance > maxSqrDist) {
                maxSqrDist = photonInfo[i].squareDistance;
            }
            
            photonEnergy = brdf->computeColor(rgbe2rgb(p.energy), -p.incomingDirection.vector(), toViewer, hitResult.hit.surfaceNormal);
            
            output += photonEnergy;
        }
        
        output = output / (M_PI * maxSqrDist);
        
    }
    else {
       output = brdf->computeColor(sourceEnergy, toLight, toViewer, hitResult.hit.surfaceNormal);
    }
    
    return output;
}

///
void CPURayTracer::setScene(std::shared_ptr<PovrayScene> scene) {
    scene_ = scene;
}
