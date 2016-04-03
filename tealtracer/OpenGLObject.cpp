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
    handle_ = 0;
    allocated_ = false;
}

///
OpenGLObject::OpenGLObject(GLuint handle) {
    handle_ = handle;
    allocated_ = true;
}

///
OpenGLObject::~OpenGLObject() {
    glFree();
}

///
bool
OpenGLObject::allocated() const {
    return allocated_;
}


///
void
OpenGLObject::glAllocate() {
    if (!allocated()) {
        allocateContent();
        allocated_ = true;
    }
}

///
void
OpenGLObject::glFree() {
    if (allocated()) {
        freeContent();
        handle_ = 0;
        allocated_ = false;
    }
}

///
GLuint
OpenGLObject::handle() const {
    return handle_;
}

///
GLuint *
OpenGLObject::handlePtr() {
    return &handle_;
}
