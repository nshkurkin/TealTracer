//
//  Raytracer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "Raytracer.hpp"

#include "TSLogger.hpp"
#include "stl_extensions.hpp"

#include "gl_include.h"
#include "OpenGLShaders.hpp"
#include "opengl_errors.hpp"

///
Raytracer::Raytracer() {
    config = RaytracingConfig();
    jobPool = JobPool(1);
    
    framesRendered = 0;
    lastRayTraceTime = 0;
    rayTraceElapsedTime = 0;
    FPSsaved = 0;
    realtimeSaved = 0;
    
    ///
    lastX = std::numeric_limits<float>::infinity();
    lastY = std::numeric_limits<float>::infinity();
}

///
void
Raytracer::setupDrawingInWindow(TSWindow * window) {
    
    TSLoggerLog(std::cout, glGetString(GL_VERSION));

    /// OpenGL
    glClearColor(0.3, 0.3, 0.3, 1.0);
    glEnable(GLenum(GL_DEPTH_TEST));
    glDepthFunc(GLenum(GL_LESS));
    
    outputImage.setDimensions(config.renderOutputWidth, config.renderOutputHeight);
    void * imageDataPtr = outputImage.dataPtr();
    target.init(outputImage.width, outputImage.height, imageDataPtr);
}

///
void
Raytracer::drawInWindow(TSWindow * window) {
    
    /// Draw the scene
    glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    target.draw();
    ///
    
    jobPool.checkAndUpdateFinishedJobs();

    float FPS = 0;
    if (rayTraceElapsedTime > 0.0001) {
        FPS = 1.0 / rayTraceElapsedTime;
    }
    
    if (framesRendered % 5 == 0 || rayTraceElapsedTime > 0.33) {
        FPSsaved = std::floor(FPS * 100.0) / 100.0;
        realtimeSaved = std::floor(rayTraceElapsedTime * 10000.0) / 10000.0;
    }
    
    /// TODO: make "title" as part of `config`
    window->setTitle(make_string(config.title, " (FPS: ", FPSsaved, ", t: ", realtimeSaved, " frames: ", framesRendered, ")"));
}

///
void
Raytracer::keyDown(TSWindow * window, int key, int scancode, int mods) {
    float transform = 1.0;
    switch (key) {
        case GLFW_KEY_A:
            if (config.controlsCamera) {
                config.scene->camera()->orientedTransform(-transform, 0, 0);
            }
            break;
        case GLFW_KEY_S:
            if (config.controlsCamera) {
                config.scene->camera()->orientedTransform(0, -transform, 0);
            }
            break;
        case GLFW_KEY_D:
            if (config.controlsCamera) {
                config.scene->camera()->orientedTransform(transform, 0, 0);
            }
            break;
        case GLFW_KEY_W:
            if (config.controlsCamera) {
                config.scene->camera()->orientedTransform(0, transform, 0);
            }
            break;
        case GLFW_KEY_Q:
            if (config.controlsCamera) {
                config.scene->camera()->orientedTransform(0, 0, -transform);
            }
            break;
        case GLFW_KEY_E:
            if (config.controlsCamera) {
                config.scene->camera()->orientedTransform(0, 0, transform);
            }
            break;
        case GLFW_KEY_B:
            TSLoggerLog(std::cout, "Breakpoint!");
            break;
        default:
            break;
    }
}

///
void
Raytracer::mouseMoved(TSWindow * window, double x, double y) {
    if (config.controlsCamera) {
        if (lastX != std::numeric_limits<float>::infinity()
         && lastY != std::numeric_limits<float>::infinity()
         && window->keyDown(GLFW_KEY_C)) {
            
            double transform = 0.1;
            double dx = x - lastX;
            double dy = y - lastY;
            
            config.scene->camera()->rotate(config.Up, -transform * dy, -transform * dx);
        }
        
        lastX = x;
        lastY = y;
    }
}

