//
//  OpenGLObject.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OpenGLObject_hpp
#define OpenGLObject_hpp

#include <memory>

#include "gl_include.h"

/// A generic object that wraps around opengl allocated data.
class OpenGLObject {
public:
    /// Creates an empty, un-allocated OpenGLObject
    OpenGLObject();
    /// Creates an OpenGLObject that is already allocated with another handle
    OpenGLObject(GLuint handle);
    ///
    virtual ~OpenGLObject();

    /// Whether or not this buffer has been allocated within opengl.
    bool allocated() const;
    
    /// Allocates opengl data for this object. In particular, allocated == true
    /// after this function is called.
    void glAllocate();
    
    /// Frees up any associated opengl data for this object. In particular,
    /// allocated == false after this function is called.
    void glFree();
    
    /// Returns the underlying OpenGL handle.
    GLuint handle() const;
    
protected:

    /// A pointer to the OpenGL handle for manipulation
    GLuint * handlePtr();

    /// Override this to actually allocate content
    virtual void allocateContent() = 0;

    /// Override this to actually free the content
    virtual void freeContent() = 0;
    
private:

    /// Whether or not this has been
    bool allocated_;
    /// The opengl handle for this object.
    GLuint handle_;

};

#endif /* OpenGLObject_hpp */
