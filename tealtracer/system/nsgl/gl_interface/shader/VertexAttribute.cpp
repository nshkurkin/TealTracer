///
/// VertexAttribute.cpp
/// ------------------
/// Nikolai Shkurkin
/// NSGL Library
///

#include "VertexAttribute.h"

using namespace nsgl;

///
VertexAttribute::VertexAttribute() {
    init("", GLint(-1), GLint(3), GLenum(GL_FLOAT));
}

VertexAttribute::VertexAttribute(std::string name) {
    init(name, GLint(-1), GLint(3), GLenum(GL_FLOAT));
}

VertexAttribute::VertexAttribute(std::string name, GLint attribLocation,
                                 GLint numElements, GLenum type) {
    init(name, attribLocation, numElements, type);
}

void VertexAttribute::init(std::string name, GLint attribLocation,
                           GLint numElements, GLenum type) {
    this->name = name;
    this->location = attribLocation;
    this->numElements = numElements;
    this->type = type;
}

void VertexAttribute::glEnable() {
    if (isValid())
        glEnableVertexAttribArray(GLuint(location));
}

void VertexAttribute::glDisable() {
    if (isValid())
        glDisableVertexAttribArray(GLuint(location));
}

void VertexAttribute::glSetLayout() {
    if (isValid()) {
        glVertexAttribPointer(GLuint(location), numElements, type,
                              GLboolean(GL_FALSE), GLsizei(0), GLvoidptr(NULL));
    }
}

bool VertexAttribute::isValid() {
    return location >= 0;
}
