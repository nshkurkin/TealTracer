//
//  OpenGLDataBuffer.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/2/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OpenGLDataBuffer.hpp"
#include "opengl_errors.hpp"

#include <cassert>

///
GLuint
OpenGLDataBufferMetaData::numBytes() const {
    return bytesPerElement * numElements;
}

///
OpenGLDataBufferMetaData::OpenGLDataBufferMetaData() {
    dataPointer = nullptr;
    bytesPerElement = 0;
    numElements = 0;
}

///
OpenGLDataBufferMetaData::OpenGLDataBufferMetaData(void * pointer, GLuint bytesPerElement, GLuint numElements) {
    this->dataPointer = pointer;
    this->bytesPerElement = bytesPerElement;
    this->numElements = numElements;
}

///
void
OpenGLDataBuffer::allocateContent() {
    glGenBuffers(1, handlePtr());
}

///
void
OpenGLDataBuffer::freeContent() {
    glDeleteBuffers(1, handlePtr());
}



///
const OpenGLDataBufferMetaData &
OpenGLDataBuffer::metaData() const {
    return metaData_;
}

///
void
OpenGLDataBuffer::setMetaData(const OpenGLDataBufferMetaData & metaData) {
    metaData_ = metaData;
    dataIsSent_ = false;
}

///
OpenGLDataBuffer::OpenGLDataBuffer(GLenum type, OpenGLDataBufferMetaData metaData) {
    this->type = type;
    this->metaData_ = metaData;
    dataIsSent_ = false;
}

///
std::shared_ptr<OpenGLDataBuffer>
OpenGLDataBuffer::arrayBuffer(const OpenGLDataBufferMetaData & metaData) {
    return std::shared_ptr<OpenGLDataBuffer>(new OpenGLDataBuffer(GLenum(GL_ARRAY_BUFFER), metaData));
}

///
std::shared_ptr<OpenGLDataBuffer>
OpenGLDataBuffer::elementArrayBuffer(const OpenGLDataBufferMetaData & metaData) {
    return std::shared_ptr<OpenGLDataBuffer>(new OpenGLDataBuffer(GLenum(GL_ELEMENT_ARRAY_BUFFER), metaData));
}

///
GLint
OpenGLDataBuffer::setAsActiveDBO() {
    glAllocate();

    GLint savedBufferBinding = 0;
    if (type == GLenum(GL_ARRAY_BUFFER)) {
        glGetIntegerv(GLenum(GL_ARRAY_BUFFER_BINDING), &savedBufferBinding);
    }
    else if (type == GLenum(GL_ELEMENT_ARRAY_BUFFER)) {
        glGetIntegerv(GLenum(GL_ELEMENT_ARRAY_BUFFER_BINDING), &savedBufferBinding);
    }

    glBindBuffer(type, handle());
    return savedBufferBinding;
}

///
void
OpenGLDataBuffer::restoreActiveDBO(GLint oldBinding) {
    glBindBuffer(type, GLuint(oldBinding));
}

///
void
OpenGLDataBuffer::sendData(GLenum usageType) {
    if (!dataIsSent_ && metaData_.numBytes() > 0) {
        /// Save the last bound buffer
        auto savedBufferBinding = setAsActiveDBO();
        glBufferData(type, GLsizeiptr(metaData_.numBytes()), metaData_.dataPointer, usageType);
        restoreActiveDBO(savedBufferBinding);
        dataIsSent_ = true;
    }
}

///
void
OpenGLDataBuffer::setNeedsUpdate() {
    dataIsSent_ = false;
}



///
void
OpenGLVertexArray::allocateContent() {
    glGenVertexArrays(1, handlePtr());
}

///
void
OpenGLVertexArray::freeContent() {
    glDeleteVertexArrays(1, handlePtr());
}


///
GLuint
OpenGLVertexArray::setAsActiveVAO() {
    glAllocate();
    
    GLint lastVAO;
    glGetIntegerv(GLenum(GL_VERTEX_ARRAY_BINDING), &lastVAO);
    glBindVertexArray(handle());
    
    return GLuint(lastVAO);
}

///
void
OpenGLVertexArray::restoreActiveVAO(GLuint oldBinding) {
    glBindVertexArray(oldBinding);
}


///
bool
OpenGLTextureMetaData::is1DTexture() const {
    return targetType == GLenum(GL_TEXTURE_1D);
}

///
bool
OpenGLTextureMetaData::is2DTexture() const {
    return targetType == GLenum(GL_TEXTURE_2D);
}

///
bool
OpenGLTextureMetaData::is3DTexture() const {
    return targetType == GLenum(GL_TEXTURE_3D);
}

///
GLint
OpenGLTextureMetaData::textureDimension() const {
    if (is1DTexture()) {
        return GLint(1);
    }
    else if (is2DTexture()) {
        return GLint(2);
    }
    else if (is3DTexture()) {
        return GLint(3);
    }
    else {
        assert(false);
        return GLint(-1);
    }
}


///
void
OpenGLTextureBuffer::allocateContent() {
    glGenTextures(1, handlePtr());
}

///
void
OpenGLTextureBuffer::freeContent() {
    glDeleteTextures(1, handlePtr());
}

    
///
GLint
OpenGLTextureBuffer::textureUnit() const {
    return GLint(textureUnit_);
}

///
void
OpenGLTextureBuffer::setTextureUnit(GLint unit) {
    textureUnit_ = unit;
}

///
const OpenGLTextureMetaData &
OpenGLTextureBuffer::metaData() const {
    return metaData_;
}
///
void OpenGLTextureBuffer::setMetaData(const OpenGLTextureMetaData & metaData) {
    metaData_ = metaData;
    dataIsSent_ = true;
}

///
OpenGLTextureBuffer::OpenGLTextureBuffer() {
    textureUnit_ = 0;
    metaData_ = OpenGLTextureMetaData();
    
    dataIsSent_ = false;
}

///
OpenGLTextureBuffer::OpenGLTextureBuffer(GLint textureUnit, const OpenGLTextureMetaData & metaData) {
    this->textureUnit_ = textureUnit;
    this->metaData_ = metaData;
    
    dataIsSent_ = false;
}

///
GLenum
OpenGLTextureBuffer::glSetActiveTextureUnit() {
    GLint lastActiveTextureUnit;
    glGetIntegerv(GLenum(GL_ACTIVE_TEXTURE), &lastActiveTextureUnit);
    glActiveTexture(textureUnit_ + GL_TEXTURE0);
    return GLenum(lastActiveTextureUnit);
}


/// Returns the texture binding type for the texture type `type`. For example,
/// type=GL_TEXTURE_1D will return GL_TEXTURE_BINDING_1D.
static inline GLenum ns_textureBindingEnumForTextureType(GLenum type) {
    GLenum bindingType = 0;
    switch (type) {
    case GLenum(GL_TEXTURE_1D):
        bindingType = GLenum(GL_TEXTURE_BINDING_1D);
        break;
    case GLenum(GL_TEXTURE_2D):
        bindingType = GLenum(GL_TEXTURE_BINDING_2D);
        break;
    case GLenum(GL_TEXTURE_3D):
        bindingType = GLenum(GL_TEXTURE_BINDING_3D);
        break;
    default:
        assert(false);
        break;
    }
    return bindingType;
}

///
OpenGLTextureBuffer::GLState
OpenGLTextureBuffer::glBind() {
    glAllocate();
    
    GLState state;
    state.lastTextureUnit = glSetActiveTextureUnit();
    state.lastBoundTexture = GLuint(0);
    
    GLint lastBoundTexture;
    GLenum bindingType = ns_textureBindingEnumForTextureType(metaData_.targetType);
    glGetIntegerv(bindingType, &lastBoundTexture);
    state.lastBoundTexture = GLuint(lastBoundTexture);
    
    glBindTexture(metaData_.targetType, handle());
    
    return state;
}

///
void
OpenGLTextureBuffer::glUnbind(const GLState & state) {
    glActiveTexture(textureUnit_ + GL_TEXTURE0);
    glBindTexture(metaData_.targetType, state.lastBoundTexture);
    
    glActiveTexture(state.lastTextureUnit);
}

///
static inline void (*ns_textureFunctionForType(GLenum type)) (const OpenGLTextureMetaData &) {
    
    void(*textureFunction)(const OpenGLTextureMetaData &) = nullptr;
    
    switch (type) {
        case GLenum(GL_TEXTURE_1D):
            textureFunction = [](const OpenGLTextureMetaData & metaData) {
                glTexImage1D(metaData.targetType, metaData.mipmapLevel, metaData.internalDataFormat, metaData.width, metaData.border, metaData.pixelFormat, metaData.pixelType, metaData.dataPointer);
            };
            break;
        case GLenum(GL_TEXTURE_2D):
            textureFunction = [](const OpenGLTextureMetaData & metaData) {
                glTexImage2D(metaData.targetType, metaData.mipmapLevel, metaData.internalDataFormat, metaData.width, metaData.height, metaData.border, metaData.pixelFormat, metaData.pixelType, metaData.dataPointer);
            };
            break;
        case GLenum(GL_TEXTURE_3D):
            textureFunction = [](const OpenGLTextureMetaData & metaData) {
                glTexImage3D(metaData.targetType, metaData.mipmapLevel, metaData.internalDataFormat, metaData.width, metaData.height, metaData.depth, metaData.border, metaData.pixelFormat, metaData.pixelType, metaData.dataPointer);
            };
            break;
        default:
            assert(false);
            break;
    }

    return textureFunction;
}

///
void
OpenGLTextureBuffer::sendData(bool deleteDataAfterSend) {
    glAllocate();

    if (!dataIsSent_) {
        auto oldBindings = glBind();
        
        ns_textureFunctionForType(metaData_.targetType)(metaData_);
        if (metaData_.mipMapped) {
            generateMipMap();
        }
        if (metaData_.linearlyInterpolated) {
            setLinearInterpolation();
        }
        
        dataIsSent_ = true;
        glUnbind(oldBindings);
    }
}

///
void
OpenGLTextureBuffer::setNeedsUpdate() {
    dataIsSent_ = false;
}

///
void
OpenGLTextureBuffer::generateMipMap() {
    if (allocated()) {
    
        auto oldBindings = glBind();
        // Generate image pyramid
        glGenerateMipmap(metaData_.targetType);
        // Set texture wrap modes for the S and T directions
        if (metaData_.textureDimension() >= 1) {
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_WRAP_S), GLfloat(GL_CLAMP_TO_EDGE));
        }
        if (metaData_.textureDimension() >= 2) {
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_WRAP_T), GLfloat(GL_CLAMP_TO_EDGE));
        }
        if (metaData_.textureDimension() >= 3) {
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_WRAP_R), GLfloat(GL_CLAMP_TO_EDGE));
        }
        
        glTexParameterfv(metaData_.targetType, GLenum(GL_TEXTURE_BORDER_COLOR), metaData_.mipMapBorderColor);
        
        glUnbind(oldBindings);
    }
}

///
void
OpenGLTextureBuffer::setLinearInterpolation() {

    if (allocated()) {
        auto oldBindings = glBind();
        
        if (metaData_.bitMapped) {
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_MAG_FILTER), GLfloat(GL_NEAREST));
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_MIN_FILTER), GLfloat(GL_NEAREST_MIPMAP_NEAREST));
        }
        else {
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_MAG_FILTER), GLfloat(GL_LINEAR));
            glTexParameterf(metaData_.targetType, GLenum(GL_TEXTURE_MIN_FILTER), GLfloat(GL_LINEAR_MIPMAP_LINEAR));
        }
        
        glUnbind(oldBindings);
    }
}
    