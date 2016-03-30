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
#include "GLFWWindow.hpp"

#include "TSLogger.hpp"
#include <cassert>

#include "gl_include.h"

class TealTracer : public TSApplication, public TSWindowDrawingDelegate {
protected:
    
    ///
    enum WindowName {
        MainWindow = 1
    };

public:

    ///
    std::shared_ptr<TSWindow> mainWindow() {
        return this->getWindow(MainWindow);
    }

    ///
    virtual void setupDrawingInWindow(TSWindow * window) {
        glClearColor(0, 0, 0, 1);
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GL_COLOR_BUFFER_BIT);
    }

protected:

    ///
    virtual int run(const std::vector<std::string> & args) {
        assert(glfwInit());
        this->createNewWindow(MainWindow);
        assert(mainWindow() != nullptr);
        mainWindow()->setDrawingDelegate(this->sharedReference<TealTracer>());
        mainWindow()->setTitle("TealTracer");
        
        while (mainWindow()->opened()) {
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
        auto window = GLFWWindow::createManaged(new GLFWWindow());
        window->setup(400, 300, "Untitled");
        return window;
    }
    
    ///
    virtual void quit() {
        mainWindow()->close();
    }
    
private:

};

#endif /* TealTracer_hpp */
