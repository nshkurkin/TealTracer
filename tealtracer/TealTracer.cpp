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

///
int
TealTracer::run(const std::vector<std::string> & args) {
    scene_ = PovrayScene::loadScene("/Users/Bo/Documents/Programming/csc490/tealtracer/tealtracer/lab1_simple.pov");
    
    assert(glfwInit());
    for (int i = 0; i < NumWindows; i++) {
        this->createNewWindow(i);
    }
    
    auto monitor = glfwGetPrimaryMonitor();
    auto videoMode = glfwGetVideoMode(monitor);
    
    gpuRayTracer_ = std::shared_ptr<GPURayTracer>(new GPURayTracer());
    gpuWindow()->setTitle("GPU Ray Tracer");
    gpuWindow()->setPosX(videoMode->width/2 - gpuWindow()->width() + 10);
    gpuWindow()->setDrawingDelegate(gpuRayTracer_);
    gpuWindow()->setEventListener(gpuRayTracer_);
    gpuRayTracer_->setScene(scene_);
    
    cpuRayTracer_ = std::shared_ptr<CPURayTracer>(new CPURayTracer());
    cpuWindow()->setTitle("CPU Ray Tracer");
    cpuWindow()->setPosX(gpuWindow()->posX() + gpuWindow()->width() + 20);
    cpuWindow()->setDrawingDelegate(cpuRayTracer_);
    cpuWindow()->setEventListener(cpuRayTracer_);
    cpuRayTracer_->setScene(scene_);
    
    while (gpuWindow()->opened()) {
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
    window->setup(400, 300, "Untitled");
    return window;
}

///
void
TealTracer::quit() {
    gpuWindow()->close();
}
