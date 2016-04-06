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

#include <atomic>

/// From Lab 1:
///
///     *) Parse the scne description file
///     *) Computing ray-object intersections
///     *) Shading
///     *) Recursive Tracing (reflection, refraction, shadows)
///     *) Write out resulting image
///

static inline Eigen::Matrix4f lookAt(const Eigen::Vector3f & eye, const Eigen::Vector3f & center, const Eigen::Vector3f & up) {
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);
    Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
    
    result(0,0) = s.x();
    result(1,0) = s.y();
    result(2,0) = s.z();
    result(0,1) = u.x();
    result(1,1) = u.y();
    result(2,1) = u.z();
    result(0,2) = -f.x();
    result(1,2) = -f.y();
    result(2,2) = -f.z();
    
    result(3,0) = -s.dot(eye);
    result(3,1) = -u.dot(eye);
    result(3,2) =  f.dot(eye);
    
    return result;
}

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
             
            -1.0, -1.0, 0.0,
             1.0,  1.0, 0.0,
            -1.0,  1.0, 0.0
        );
        target.texcoords = make_vector<GLfloat>(
            0.0, 0.0,
            1.0, 0.0,
            1.0, 1.0,
            
            0.0, 0.0,
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

        outputImage.setDimensions(256, 256, Image::Vector4ub(255, 255, 255, 255));
        auto textureFormat = OpenGLTextureMetaData();
        
        textureFormat.targetType = GL_TEXTURE_2D;
        textureFormat.pixelFormat = GL_RGBA;
        textureFormat.pixelType = GL_UNSIGNED_BYTE;
        textureFormat.internalDataFormat = GL_RGBA;
        textureFormat.width = outputImage.width;
        textureFormat.height = outputImage.height;
        textureFormat.linearlyInterpolated = true;
        textureFormat.mipMapped = true;
        textureFormat.dataPointer = outputImage.dataPtr();
        
        target.outputTexture = std::shared_ptr<OpenGLTextureBuffer>(new OpenGLTextureBuffer(0, textureFormat));
        target.outputTexture->sendData();
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        auto lastProgramHandle = target.program->setAsActiveProgram();
        
        auto oldTextureState = target.outputTexture->glBind();
        target.outputTexture->sendData();
        target.program->attach("tex", target.outputTexture.get());
        
        auto oldVaoHandle = target.triangleVAO->setAsActiveVAO();
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
        if (key == GLFW_KEY_R) {
            raytraceScene();
        }
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
    
    static const Eigen::Vector3f Up;
    static const Eigen::Vector3f Forward;
    static const Eigen::Vector3f Right;
    
    ///
    void raytraceScene() {
        assert(scene_ != nullptr);
        
        /// Get the camera
        auto camera = scene_->camera();
        
        /// TODO: build the photon map
        
        /// Create all of the rays
        auto camPos = camera->location();
        auto viewTransform = lookAt(camera->location(), camera->lookAt(), camera->up());
        Eigen::Vector3f forward = (viewTransform * Eigen::Vector4f(Forward.x(), Forward.y(), Forward.z(), 0.0)).block<3,1>(0,0);
        Eigen::Vector3f up = (viewTransform * Eigen::Vector4f(Up.x(), Up.y(), Up.z(), 0.0)).block<3,1>(0,0);
        Eigen::Vector3f right = (viewTransform * Eigen::Vector4f(Right.x(), Right.y(), Right.z(), 0.0)).block<3,1>(0,0);
        
        for (int px = 0; px < outputImage.width; px++) {
            for (int py = 0; py < outputImage.height; py++) {
                Ray ray;
                ray.origin = camPos;
                Eigen::Vector3f pixelPos = camPos + forward - 0.5*up - 0.5*right + right*(0.5+(double)px)/(double)outputImage.width + up*(0.5+(double)py)/(double)outputImage.height;
                ray.direction = (pixelPos - camPos).normalized();
                
                auto hitTest = scene_->closestIntersection(ray);
                Image::Vector4ub color = Image::Vector4ub(100, 100, 100, 255);
                
                if (hitTest.element != nullptr) {
                    color = Image::Vector4ub(200, 0, 0, 255);
                }
                
                outputImage.pixel(px, py) = color;
            }
        }
        
        target.outputTexture->setNeedsUpdate();
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
