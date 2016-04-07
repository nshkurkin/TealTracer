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
#include "TextureRenderTarget.hpp"

class GPURayTracer : public TSWindowDrawingDelegate, public TSUserEventListener {
public:

    ///
    /// Rendering pipeline setup:
    ///     1) Have an OpenGL texture to display on screen, connect it to GPU
    ///     2) Perform raycasts on the GPU and fill the texture with content
    ///     3) Display the texture
    ///
    
    ComputeEngine computeEngine;
    TextureRenderTarget target;

    ///
    virtual void setupDrawingInWindow(TSWindow * window) {
        
        TSLoggerLog(std::cout, glGetString(GL_VERSION));
    
        /// OpenGL
        glClearColor(0.3, 0.3, 0.3, 1.0);
        glEnable(GLenum(GL_DEPTH_TEST));
        glDepthFunc(GLenum(GL_LESS));
        
        std::vector<uint8_t> empty;
        empty.resize(window->width() * window->height() * 4, 100);
        target.init(window->width(), window->height(), &empty[0]);
        
        /// OpenCL
        ocl_computeSquares(); // test code
    }

    ///
    virtual void drawInWindow(TSWindow * window) {
        glClear(GLbitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        target.draw();
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
    
    /// Tests the usage on "ComputeEngine" following the example given at the
    /// following web address:
    ///     https://developer.apple.com/library/mac/samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html
    void ocl_computeSquares() {
    
        /// OpenCL
        computeEngine.connect(ComputeEngine::DEVICE_TYPE_GPU, 1, true);
        
        /// Copy the elements of the scene onto the GPU
        unsigned int elementCount = 10000;
        
        computeEngine.createProgramFromFile("square_prog", "sample_square.cl");
        computeEngine.createKernel("square_prog", "square");
        computeEngine.createBuffer("square_input", ComputeEngine::MemFlags::MEM_READ_ONLY, sizeof(float) * elementCount);
        computeEngine.createBuffer("square_output", ComputeEngine::MemFlags::MEM_WRITE_ONLY, sizeof(float) * elementCount);
        
        std::vector<float> values;
        for (int i = 0; i < elementCount; i++) {
            values.push_back(float(i));
        }
        computeEngine.writeBuffer("square_input", 0, 0, sizeof(float) * elementCount, &values[0]);
        
        computeEngine.setKernelArg("square", 0, &computeEngine.getBuffer("square_input"), sizeof(cl_mem));
        computeEngine.setKernelArg("square", 1, &computeEngine.getBuffer("square_output"), sizeof(cl_mem));
        computeEngine.setKernelArg("square", 2, &elementCount, sizeof(unsigned int));
        
        size_t globalCount = elementCount;
        size_t localCount = 1000; //computeEngine.getEstimatedWorkGroupSize("square", 0); == 1024?
        computeEngine.executeKernel("square", 0, &globalCount, &localCount, 1);
        computeEngine.finish(0);
        
        std::vector<float> output(elementCount, 1.0);
        computeEngine.readBuffer("square_output", 0, 0, sizeof(float) * elementCount, &output[0]);
        
        for (int i = 0; i < elementCount; i++) {
            assert(output[i] == values[i] * values[i]);
        }
        
        computeEngine.disconnect();
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
