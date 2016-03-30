//
//  OpenGLObject.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OpenGLObject.hpp"

///
OpenGLObject::OpenGLObject() {
    handle_ = nullptr;
}

///
OpenGLObject::OpenGLObject(GLuint handle) {
    handle_ = std::shared_ptr<GLuint>(new GLuint);
    *handle_ = handle;
}

///
OpenGLObject::~OpenGLObject() {
    glFree();
}

/// Whether or not this buffer has been allocated within opengl.
bool OpenGLObject::allocated() const {return handle_ != nullptr;}


/// Allocates opengl data for this object. In particular, allocated == true
/// after this function is called.
void OpenGLObject::glAllocate() {
    if (!allocated()) {
        handle_ = allocateContent();
    }
}

/// Frees up any associated opengl data for this object. In particular,
/// allocated == false after this function is called.
void OpenGLObject::glFree() {
    if (allocated()) {
        freeContent();
        handle_ = nullptr;
    }
}

/// Returns the underlying OpenGL handle.
GLuint OpenGLObject::handle() const {
    return *handle_;
}
