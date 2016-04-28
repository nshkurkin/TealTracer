//
//  TealTracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TealTracer.hpp"
#include "TSLogger.hpp"


///
enum WindowName {
    CPUWindow = 0,
    GPUWindow,
    NumWindows
};

///
std::shared_ptr<TSWindow>
TealTracer::gpuWindow() {
    return this->getWindow(GPUWindow);
}

///
std::shared_ptr<TSWindow>
TealTracer::cpuWindow() {
    return this->getWindow(CPUWindow);
}

#include "json.hpp"
using json = nlohmann::json;

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
    
    scene_ = PovrayScene::loadScene(config["povrayScene"].get<std::string>());
    
//    scene_->writeOut(std::cout);
    
    gpuRayTracer_ = std::shared_ptr<GPURayTracer>(new GPURayTracer());
    
    gpuRayTracer_->setScene(scene_);
    gpuRayTracer_->renderOutputWidth = config["GPURayTracer"]["outputWidth"].get<int>();
    gpuRayTracer_->renderOutputHeight = config["GPURayTracer"]["outputHeight"].get<int>();
    gpuRayTracer_->useGPU = config["GPURayTracer"]["useGPU"].get<bool>();
    gpuRayTracer_->brdfType = (GPURayTracer::SupportedBRDF) config["GPURayTracer"]["brdfType"].get<int>();
    gpuRayTracer_->numberOfPhotonsToGather = config["GPURayTracer"]["numberOfPhotonsToGather"].get<int>();
    gpuRayTracer_->raysPerLight = config["GPURayTracer"]["raysPerLight"].get<int>();
    gpuRayTracer_->lumensPerLight = config["GPURayTracer"]["lumensPerLight"].get<int>();
    gpuRayTracer_->photonBounceProbability = config["GPURayTracer"]["photonBounceProbability"].get<double>();
    gpuRayTracer_->photonBounceEnergyMultipler = config["GPURayTracer"]["photonBounceEnergyMultipler"].get<double>();
    
    gpuWindow()->setTitle(config["GPURayTracer"]["initialTitle"].get<std::string>());
    gpuWindow()->setWidth(config["windowWidth"].get<int>());
    gpuWindow()->setHeight(config["windowHeight"].get<int>());
    gpuWindow()->setPosX(videoMode->width/2 - gpuWindow()->width() + 10);
    gpuWindow()->setDrawingDelegate(gpuRayTracer_);
    gpuWindow()->setEventListener(gpuRayTracer_);
    
    cpuRayTracer_ = std::shared_ptr<CPURayTracer>(new CPURayTracer());
    
    cpuRayTracer_->setScene(scene_);
    cpuRayTracer_->renderOutputWidth = config["CPURayTracer"]["outputWidth"].get<int>();
    cpuRayTracer_->renderOutputHeight = config["CPURayTracer"]["outputHeight"].get<int>();
    cpuRayTracer_->numberOfPhotonsToGather = config["CPURayTracer"]["numberOfPhotonsToGather"].get<int>();
    cpuRayTracer_->raysPerLight = config["CPURayTracer"]["raysPerLight"].get<int>();
    cpuRayTracer_->lumensPerLight = config["CPURayTracer"]["lumensPerLight"].get<int>();
    cpuRayTracer_->photonMapType = (CPURayTracer::SupportedPhotonMap) config["CPURayTracer"]["photonMapType"].get<int>();
    cpuRayTracer_->brdfType = (CPURayTracer::SupportedBRDF) config["CPURayTracer"]["brdfType"].get<int>();
    cpuRayTracer_->photonBounceProbability = config["CPURayTracer"]["photonBounceProbability"].get<double>();
    cpuRayTracer_->photonBounceEnergyMultipler = config["CPURayTracer"]["photonBounceEnergyMultipler"].get<double>();
    cpuRayTracer_->mapShadowPhotons = config["CPURayTracer"]["mapShadowPhotons"].get<bool>();
    
    cpuWindow()->setTitle(config["CPURayTracer"]["initialTitle"].get<std::string>());
    cpuWindow()->setWidth(config["windowWidth"].get<int>());
    cpuWindow()->setHeight(config["windowHeight"].get<int>());
    cpuWindow()->setPosX(gpuWindow()->posX() + gpuWindow()->width() + 20);
    cpuWindow()->setDrawingDelegate(cpuRayTracer_);
    cpuWindow()->setEventListener(cpuRayTracer_);
    
    if (config["GPURayTracer"]["enabled"].get<bool>()) {
        gpuRayTracer_->start();
    }
    if (config["CPURayTracer"]["enabled"].get<bool>()) {
        cpuRayTracer_->start();
    }
    
    while (gpuWindow()->opened() && cpuWindow()->opened()) {
        glfwPollEvents();
        for (auto windowItr = windowsBegin(); windowItr != windowsEnd(); windowItr++) {
            windowItr->second->draw();
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
    gpuWindow()->close();
}
