//
//  TealTracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TealTracer.hpp"
#include "TSLogger.hpp"

#include <cassert>

#include "json.hpp"
using json = nlohmann::json;

#include "gl_include.h"
#include "RaytracingConfig.hpp"

#include "SCMonteCarloRaytracer.hpp" // Single Core: Direct
#include "SCKDTreeRaytracer.hpp" // Single Core: KDTree
#include "SCHashGridRaytracer.hpp" // Single Core: HashGrid
#include "SCTilePhotonRaytracer.hpp" // Single Core: Tiled


#include "OCLMonteCarloRaytracer.hpp" // OpenCL: Direct
/// No OpenCL KDTree implementation
#include "OCLPhotonHashGridRaytracer.hpp" // OpenCL: HashGrid
#include "OCLOptimizedHashGridRaytracer.hpp" // OpenCL: HashGrid Variant
#include "OCLTiledPhotonRaytracer.hpp" // OpenCL: Tiled
#include "OCLOptimizedTiledPhotonRaytracer.hpp" // OpenCL: Tiled Variant



///
enum WindowName {
    RightWindow = 0,
    LeftWindow,
    NumWindows
};

///
std::shared_ptr<TSWindow>
TealTracer::leftWindow() {
    return this->getWindow(LeftWindow);
}

///
std::shared_ptr<TSWindow>
TealTracer::rightWindow() {
    return this->getWindow(RightWindow);
}

///
int
TealTracer::run(const std::vector<std::string> & args) {
    assert(glfwInit());
    for (int i = 0; i < NumWindows; i++) {
        this->createNewWindow(i);
    }
    
    std::string file("config.json");
    std::ifstream source;
    source.open(file.c_str(), std::ios_base::in);
    if (!source) {
        TSLoggerLog(std::cout, "could not find file=", file);
        assert(false);
    }
    
    std::string content;
    std::string line;
    while (!std::getline(source, line).eof()) {
        /// Remove all comments
        content.append(line);
    }
    
    auto monitor = glfwGetPrimaryMonitor();
    auto videoMode = glfwGetVideoMode(monitor);
    json config = json::parse(content);
    
    Eigen::Vector3f Up, Forward, Right;
    Up = RaytracingConfig::vec3FromData(config["Up"].get<std::vector<double>>());
    Forward = RaytracingConfig::vec3FromData(config["Forward"].get<std::vector<double>>());
    Right = RaytracingConfig::vec3FromData(config["Right"].get<std::vector<double>>());
    
    std::map<std::string, std::shared_ptr<Raytracer>> availableRaytracers;
    
    availableRaytracers["SCMonteCarloRaytracer"] = std::shared_ptr<SCMonteCarloRaytracer>(new SCMonteCarloRaytracer());
    availableRaytracers["SCKDTreeRaytracer"] = std::shared_ptr<SCKDTreeRaytracer>(new SCKDTreeRaytracer());
    availableRaytracers["SCHashGridRaytracer"] = std::shared_ptr<SCHashGridRaytracer>(new SCHashGridRaytracer());
    availableRaytracers["SCTilePhotonRaytracer"] = std::shared_ptr<SCTilePhotonRaytracer>(new SCTilePhotonRaytracer());
    
    availableRaytracers["OCLMonteCarloRaytracer"] = std::shared_ptr<OCLMonteCarloRaytracer>(new OCLMonteCarloRaytracer());
    availableRaytracers["OCLPhotonHashGridRaytracer"] = std::shared_ptr<OCLPhotonHashGridRaytracer>(new OCLPhotonHashGridRaytracer());
    availableRaytracers["OCLOptimizedHashGridRaytracer"] = std::shared_ptr<OCLOptimizedHashGridRaytracer>(new OCLOptimizedHashGridRaytracer());
    availableRaytracers["OCLTiledPhotonRaytracer"] = std::shared_ptr<OCLTiledPhotonRaytracer>(new OCLTiledPhotonRaytracer());
    availableRaytracers["OCLOptimizedTiledPhotonRaytracer"] = std::shared_ptr<OCLOptimizedTiledPhotonRaytracer>(new OCLOptimizedTiledPhotonRaytracer());
    
    std::string leftRaytracerName = config["LeftRaytracer"]["name"].get<std::string>();
    std::string leftRaytracerConfigName = config["LeftRaytracer"]["config"].get<std::string>();
    std::string rightRaytracerName = config["RightRaytracer"]["name"].get<std::string>();
    std::string rightRaytracerConfigName = config["RightRaytracer"]["config"].get<std::string>();
    
    leftRaytracer_ = availableRaytracers[leftRaytracerName];
    rightRaytracer_ = availableRaytracers[rightRaytracerName];
    
    /// Assuming we have two GPUs availabled, we can allow the OpenCL raytracers
    ///     to work independently of one another. This works on my machine because
    ///     I have 2 GPUs :)
    if (dynamic_cast<OpenCLRaytracer *>(leftRaytracer_.get()) != nullptr
     && dynamic_cast<OpenCLRaytracer *>(rightRaytracer_.get()) != nullptr) {
        dynamic_cast<OpenCLRaytracer *>(leftRaytracer_.get())->activeDevice = 0;
        dynamic_cast<OpenCLRaytracer *>(rightRaytracer_.get())->activeDevice = 1;
    }
    
    leftRaytracer_->config.loadFromJSON(config[leftRaytracerConfigName]);
    rightRaytracer_->config.loadFromJSON(config[rightRaytracerConfigName]);
    
    leftRaytracer_->config.title = config[leftRaytracerName]["screenName"].get<std::string>() + " " +  leftRaytracer_->config.title;
    rightRaytracer_->config.title = config[rightRaytracerName]["screenName"].get<std::string>() + " " +  rightRaytracer_->config.title;
    
    scene_ = PovrayScene::loadScene(config["povrayScene"].get<std::string>());
    leftRaytracer_->config.Up = Up;
    leftRaytracer_->config.Forward = Forward;
    leftRaytracer_->config.Right = Right;
    leftRaytracer_->config.scene = scene_;
    
    rightRaytracer_->config.Up = Up;
    rightRaytracer_->config.Forward = Forward;
    rightRaytracer_->config.Right = Right;
    rightRaytracer_->config.scene = scene_;
    
    leftWindow()->setTitle(leftRaytracer_->config.title);
    leftWindow()->setWidth(config["windowWidth"].get<int>());
    leftWindow()->setHeight(config["windowHeight"].get<int>());
    leftWindow()->setPosX(videoMode->width/2 - leftWindow()->width() + 10);
    leftWindow()->setDrawingDelegate(leftRaytracer_);
    leftWindow()->setEventListener(leftRaytracer_);
    
    rightWindow()->setTitle(rightRaytracer_->config.title);
    rightWindow()->setWidth(config["windowWidth"].get<int>());
    rightWindow()->setHeight(config["windowHeight"].get<int>());
    rightWindow()->setPosX(leftWindow()->posX() + leftWindow()->width() + 20);
    rightWindow()->setDrawingDelegate(rightRaytracer_);
    rightWindow()->setEventListener(rightRaytracer_);
    
    if (leftRaytracer_->config.enabled) {
        leftRaytracer_->start();
    }
    if (rightRaytracer_->config.enabled) {
        rightRaytracer_->start();
    }
    
    TSLoggerLog(std::cout, glGetString(GL_VERSION));
    
    while (leftWindow()->opened() && rightWindow()->opened()) {
        double startTime = glfwGetTime();
//        TSLoggerLog(std::cout, "Starting iteration");
        glfwPollEvents();
        for (auto windowItr = windowsBegin(); windowItr != windowsEnd(); windowItr++) {
            windowItr->second->draw();
        }
        double endTime = glfwGetTime();
        double dt = (endTime - startTime); // dt = time in milliseconds
//        TSLoggerLog(std::cout, "Time elapsed=", dt);
        if (dt < (1.0 / 60.0)) {
            int toElapse = 1000.0 * ((1.0 / 60.0) - dt);
            usleep(toElapse);
        }
    }
    
    glfwTerminate();
    return 0;
}

///
std::shared_ptr<TSWindow>
TealTracer::newWindow() {
    auto window = Window::createManaged(new Window());
    window->setup(100, 100, "Untitled");
    return window;
}

///
void
TealTracer::quit() {
    leftWindow()->close();
}
