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
#include "opengl_errors.hpp"
#include "compute_engine.hpp"
#include "stl_extensions.hpp"

#include "OpenGLShaders.hpp"
#include "PovrayScene.hpp"
#include "TSLogger.hpp"
#include "Image.hpp"
#include "JobPool.hpp"

/// From Lab 1:
///
///     *) Parse the scne description file
///     *) Computing ray-object intersections
///     *) Shading
///     *) Recursive Tracing (reflection, refraction, shadows)
///     *) Write out resulting image
///

Eigen::Matrix4f lookAt(const Eigen::Vector3f & eye, const Eigen::Vector3f & center, const Eigen::Vector3f & up);

///
class CPURayTracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:
    
    ///
    /// Rendering pipeline:
    ///
    ///     1) Queue a Raytrace into "outputImage"
    ///     2) When complete, copy data from "outputImage" to "outputTexture"
    ///     3) Render "outputTexture"
    ///
    struct RenderTarget {
        std::shared_ptr<OpenGLTextureBuffer> outputTexture;
        std::shared_ptr<OpenGLProgram> program;
        
        std::vector<GLfloat> points;
        std::vector<GLfloat> texcoords;
        
        std::shared_ptr<OpenGLVertexArray> triangleVAO;
        std::shared_ptr<OpenGLDataBuffer> positionDBO;
        std::shared_ptr<OpenGLDataBuffer> texcoordDBO;
    };

    Image outputImage;
    RenderTarget target;
    JobPool jobPool;

    ///
    virtual void setupDrawingInWindow(TSWindow * window);

    int framesRendered;
    double lastRayTraceTime, rayTraceElapsedTime;

    ///
    void enqueRayTrace();

    bool firstDraw;

    ///
    virtual void drawInWindow(TSWindow * window);
    
    ///
    virtual void windowResize(TSWindow * window, int w, int h);
    ///
    virtual void framebufferResize(TSWindow * window, int w, int h);
    ///
    virtual void windowClose(TSWindow * window);
    
    ///
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods);
    ///
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods);
    ///
    virtual void mouseUp(TSWindow * window, int button, int mods);
    ///
    virtual void mouseDown(TSWindow * window, int button, int mods);
    ///
    virtual void mouseMoved(TSWindow * window, double x, double y);
    ///
    virtual void mouseScroll(TSWindow * window, double dx, double dy);
    
    static const Eigen::Vector3f Up;
    static const Eigen::Vector3f Forward;
    static const Eigen::Vector3f Right;
    
    ///
    void raytraceScene();
    
protected:

    friend class TealTracer;

    ///
    void setScene(std::shared_ptr<PovrayScene> scene);
    
private:
    ///
    std::shared_ptr<PovrayScene> scene_;

    
};

#endif /* CPURayTracer_hpp */
