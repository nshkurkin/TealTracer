//
//  Raytracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef Raytracer_hpp
#define Raytracer_hpp

#include "TSWindow.hpp"
#include "JobPool.hpp"

#include "Image.hpp"
#include "TextureRenderTarget.hpp"
#include "RaytracingConfig.hpp"
#include "PovrayScene.hpp"

#include "TSRandomValueGenerator.hpp"

class Raytracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:

    ///
    /// Rendering pipeline setup:
    ///     1) Have an OpenGL texture to display on screen
    ///     2) Perform raycasts and fill the texture with content
    ///     3) Display the texture
    ///

    RaytracingConfig config;

    Raytracer();

    /// Override this to provide ray tracing functionality
    virtual void start() {}
    
    virtual void setupDrawingInWindow(TSWindow * window);
    virtual void drawInWindow(TSWindow * window);
    
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods);
    virtual void mouseMoved(TSWindow * window, double x, double y);
    

    virtual void windowResize(TSWindow * window, int w, int h) {}
    virtual void framebufferResize(TSWindow * window, int w, int h) {}
    virtual void windowClose(TSWindow * window) {}
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods) {}
    virtual void mouseUp(TSWindow * window, int button, int mods) {}
    virtual void mouseDown(TSWindow * window, int button, int mods) {}
    virtual void mouseScroll(TSWindow * window, double dx, double dy) {}

protected:

    JobPool jobPool;
    TSRandomValueGenerator generator;
    
    TextureRenderTarget target;
    Image<uint8_t> outputImage;

    int framesRendered;
    double lastRayTraceTime, rayTraceElapsedTime;
    float FPSsaved, realtimeSaved;
    
    double lastX, lastY;

};

#endif /* Raytracer_hpp */
