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
    ///
    OpenGLObject();
    ///
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

    /// Override this to actuall allocated content
    virtual std::shared_ptr<GLuint> allocateContent() = 0;

    /// Override this to actually free the content
    virtual void freeContent() = 0;
    
private:

    /// The opengl handle for this object.
    std::shared_ptr<GLuint> handle_;

};

#endif /* OpenGLObject_hpp */
