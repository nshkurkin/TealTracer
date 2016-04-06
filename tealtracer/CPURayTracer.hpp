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
    std::shared_ptr<PovrayScene> scene;
    RenderTarget target;

    ///
    virtual void setupDrawingInWindow(TSWindow * window) {
        ///
        glClearColor(0.3, 0.3, 0.3, 1.0);
        glEnable(GLenum(GL_DEPTH_TEST));
        glDepthFunc(GLenum(GL_LESS));
        
        target.points = make_vector<GLfloat>(
            -1.0, -1.0, 0.0,
             1.0, -1.0, 0.0,
             1.0,  1.0, 0.0,
             
             1.0, -1.0, 0.0,
             1.0,  1.0, 0.0,
            -1.0,  1.0, 0.0
        );
        target.texcoords = make_vector<GLfloat>(
            0.0, 0.0,
            1.0, 0.0,
            1.0, 1.0,
            
            1.0, 0.0,
            1.0, 1.0,
            0.0, 1.0
        );
            
        ///
        target.positionDBO = OpenGLDataBuffer::arrayBuffer(OpenGLDataBufferMetaData(&target.points[0], GLuint(sizeof(GLfloat) * 3), GLuint(target.points.size())));
        target.positionDBO->sendData(GLenum(GL_STATIC_DRAW));
        
        target.texcoordDBO = OpenGLDataBuffer::arrayBuffer(OpenGLDataBufferMetaData(&target.texcoords[0], GLuint(sizeof(GLfloat) * 2), GLuint(target.texcoords.size())));
        target.texcoordDBO->sendData(GLenum(GL_STATIC_DRAW));

        target.program = std::shared_ptr<OpenGLProgram>(new OpenGLProgram());
        target.program->shaders = make_vector<std::shared_ptr<OpenGLShader>>(
            OpenGLShader::vertexShaderWithFilePath("texture.vert.glsl"),
            OpenGLShader::fragmentShaderWithFilePath("texture.frag.glsl")
        );
        
        target.triangleVAO = std::shared_ptr<OpenGLVertexArray>(new OpenGLVertexArray());
        
        target.program->build(true);
        target.program->connectDataToProgram(target.triangleVAO.get(), make_map<std::string, OpenGLDataBuffer*>(
            "vertex_position", target.positionDBO.get(),
            "vertex_texcoord", target.texcoordDBO.get()
        ));

        outputImage.setDimensions(400, 300);
        OpenGLTextureMetaData textureFormat;
        
        textureFormat.targetType = GL_TEXTURE_2D;
        textureFormat.mipmapLevel = 0;
        textureFormat.pixelFormat = GL_RGBA;
        textureFormat.pixelType = GL_UNSIGNED_BYTE;
        textureFormat.internalDataFormat = GL_RGBA;
        textureFormat.width = outputImage.width;
        textureFormat.height = outputImage.height;
        textureFormat.dataPointer = outputImage.dataPtr();
        textureFormat.mipMapped = false;
        textureFormat.bitMapped = false;
        
        target.outputTexture = std::shared_ptr<OpenGLTextureBuffer>(new OpenGLTextureBuffer(0, textureFormat));
        target.outputTexture->sendData(GLenum(GL_DYNAMIC_DRAW));
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        auto lastProgramHandle = target.program->setAsActiveProgram();
        auto oldVaoHandle = target.triangleVAO->setAsActiveVAO();
        auto oldTextureState = target.outputTexture->glBind();
        
        target.program->attach("tex", target.outputTexture.get());
        glDrawArrays(GLenum(GL_TRIANGLES), 0, GLsizei(target.points.size()));
        
        target.outputTexture->glUnbind(oldTextureState);
        target.triangleVAO->restoreActiveVAO(oldVaoHandle);
        target.program->restoreActiveProgram(lastProgramHandle);
    }

    
    ///
    virtual void windowResize(TSWindow * window, int w, int h) {
    
    }
    
    ///
    virtual void framebufferResize(TSWindow * window, int w, int h) {
        glViewport(0, 0, GLsizei(w), GLsizei(h));
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
