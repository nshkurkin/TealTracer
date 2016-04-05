//
//  GPURayTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef GPURayTracer_hpp
#define GPURayTracer_hpp

#include "TSWindow.hpp"
#include "gl_include.h"
#include "opengl_errors.hpp"
#include "compute_engine.hpp"
#include "stl_extensions.hpp"

#include "OpenGLShaders.hpp"
#include "PovrayScene.hpp"
#include "TSLogger.hpp"

class GPURayTracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:

    ///
    struct Scene {
        std::vector<GLfloat> points;
        std::vector<GLfloat> colors;
        
        std::shared_ptr<OpenGLVertexArray> triangleVAO;
        std::shared_ptr<OpenGLDataBuffer> positionDBO;// = OpenGLDataBufferObject.arrayBuffer()
        std::shared_ptr<OpenGLDataBuffer> colorDBO; // = OpenGLDataBufferObject.arrayBuffer()
    };
    
    ComputeEngine computeEngine;// = OpenCLComputeEngine(useOpenGLContext: true)
    std::shared_ptr<OpenGLProgram> gpuGLProgram; //: OpenGLProgram? = nil
    Scene scene;

    ///
    virtual void setupDrawingInWindow(TSWindow * window) {
        
        TSLoggerLog(std::cout, glGetString(GL_VERSION));
        
        ///
        computeEngine.connect(ComputeEngine::DEVICE_TYPE_GPU, 2, true);
    
        ///
        glClearColor(0.3, 0.3, 0.3, 1.0);
        glEnable(GLenum(GL_DEPTH_TEST));
        glDepthFunc(GLenum(GL_LESS));
        
        scene.points = make_vector<GLfloat>(
            0.0, 0.5, 0.0,
            0.5, -0.5, 0.0,
            -0.5, -0.5, 0.0
        );
        scene.colors = make_vector<GLfloat>(
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        );
            
        /// http://antongerdelan.net/opengl/vertexbuffers.html
        scene.positionDBO = OpenGLDataBuffer::arrayBuffer(OpenGLDataBufferMetaData(&scene.points[0], GLuint(sizeof(GLfloat) * 3), GLuint(scene.points.size())));
        scene.positionDBO->sendData(GLenum(GL_STATIC_DRAW));
        
        scene.colorDBO = OpenGLDataBuffer::arrayBuffer(OpenGLDataBufferMetaData(&scene.colors[0], GLuint(sizeof(GLfloat) * 3), GLuint(scene.colors.size())));
        scene.colorDBO->sendData(GLenum(GL_STATIC_DRAW));

        gpuGLProgram = std::shared_ptr<OpenGLProgram>(new OpenGLProgram());
        gpuGLProgram->shaders = make_vector<std::shared_ptr<OpenGLShader>>(
            OpenGLShader::vertexShaderWithFilePath("SampleVertexShader.glsl"),
            OpenGLShader::fragmentShaderWithFilePath("SampleFragmentShader.glsl")
        );
        
        scene.triangleVAO = std::shared_ptr<OpenGLVertexArray>(new OpenGLVertexArray());
        
        gpuGLProgram->build(true);
        gpuGLProgram->connectDataToProgram(scene.triangleVAO.get(), make_map<std::string, OpenGLDataBuffer*>(
            "vertex_position", scene.positionDBO.get(),
            "vertex_color", scene.colorDBO.get()
        ));
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        auto lastProgramHandle = gpuGLProgram->setAsActiveProgram();
        auto oldVaoHandle = scene.triangleVAO->setAsActiveVAO();
        
        glDrawArrays(GLenum(GL_TRIANGLES), 0, GLsizei(scene.points.size()));
        
        scene.triangleVAO->restoreActiveVAO(oldVaoHandle);
        gpuGLProgram->restoreActiveProgram(lastProgramHandle);
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

#endif /* GPURayTracer_hpp */
