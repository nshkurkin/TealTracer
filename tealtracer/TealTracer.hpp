//
//  TealTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TealTracer_hpp
#define TealTracer_hpp

#include "TSApplication.hpp"
#include "TSLogger.hpp"

#include <cassert>

#include "gl_include.h"
#include "Window.hpp"
#include "GPURayTracer.hpp"
#include "CPURayTracer.hpp"

class TealTracer : public TSApplication {
protected:
    
    ///
    enum WindowName {
        CPUWindow = 0,
        GPUWindow,
        NumWindows
    };

public:

    ///
    std::shared_ptr<TSWindow> gpuWindow() {
        return this->getWindow(GPUWindow);
    }
    
    ///
    std::shared_ptr<TSWindow> cpuWindow() {
        return this->getWindow(CPUWindow);
    }

protected:

    ///
    virtual int run(const std::vector<std::string> & args) {
        assert(glfwInit());
        for (int i = 0; i < NumWindows; i++) {
            this->createNewWindow(i);
        }
        
        auto monitor = glfwGetPrimaryMonitor();
        auto videoMode = glfwGetVideoMode(monitor);
        
        gpuRayTracer = std::shared_ptr<GPURayTracer>(new GPURayTracer());
        gpuWindow()->setDrawingDelegate(gpuRayTracer);
        gpuWindow()->setEventListener(gpuRayTracer);
        gpuWindow()->setTitle("GPU Ray Tracer");
        gpuWindow()->setPosX(videoMode->width/2 - gpuWindow()->width() + 10);
        
        cpuRayTracer = std::shared_ptr<CPURayTracer>(new CPURayTracer());
        cpuWindow()->setDrawingDelegate(cpuRayTracer);
        cpuWindow()->setEventListener(cpuRayTracer);
        cpuWindow()->setTitle("CPU Ray Tracer");
        cpuWindow()->setPosX(gpuWindow()->posX() + gpuWindow()->width() + 20);
        
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
    virtual std::shared_ptr<TSWindow> newWindow() {
        auto window = Window::createManaged(new Window());
        window->setup(400, 300, "Untitled");
        return window;
    }
    
    ///
    virtual void quit() {
        gpuWindow()->close();
    }
    
private:

    std::shared_ptr<GPURayTracer> gpuRayTracer;
    std::shared_ptr<CPURayTracer> cpuRayTracer;

};

#endif /* TealTracer_hpp */
