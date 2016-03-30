//
//  CPURayTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef CPURayTracer_hpp
#define CPURayTracer_hpp

#include "TSWindow.hpp"
#include "gl_include.h"

#include "PovrayScene.hpp"

/// From Lab 1:
///
///     *) Parse the scne description file
///     *) Computing ray-object intersections
///     *) Shading
///     *) Recursive Tracing (reflection, refraction, shadows)
///     *) Write out resulting image
///

///
class CPURayTracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:

    ///
    virtual void setupDrawingInWindow(TSWindow * window) {
        glClearColor(0, 0, 0, 1);
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    
    ///
    virtual void windowResize(TSWindow * window, int w, int h) {
    
    }
    
    ///
    virtual void framebufferResize(TSWindow * window, int w, int h) {
    
    }

    ///
    virtual void windowClose(TSWindow * window) {
    
    }
    
    ///
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods) {
    
    }
    
    ///
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods) {
    
    }
    
    ///
    virtual void mouseUp(TSWindow * window, int button, int mods) {
    
    }
    
    ///
    virtual void mouseDown(TSWindow * window, int button, int mods) {
    
    }
    
    ///
    virtual void mouseMoved(TSWindow * window, double x, double y) {
    
    }
    
    ///
    virtual void mouseScroll(TSWindow * window, double dx, double dy) {
    
    }
    
protected:

    friend class TealTracer;

    ///
    void setScene(std::shared_ptr<PovrayScene> scene) {
        scene_ = scene;
    }
    
private:
    ///
    std::shared_ptr<PovrayScene> scene_;

    
};

#endif /* CPURayTracer_hpp */
