//
//  OpenGLDataBuffer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/2/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OpenGLDataBuffer_hpp
#define OpenGLDataBuffer_hpp

#include "gl_include.h"
#include "OpenGLObject.hpp"

///
struct OpenGLDataBufferMetaData {
    
    ///
    GLuint bytesPerElement;
    ///
    GLuint numElements;
    ///
    void * dataPointer;
    
    /// The total number of bytes pointed to by `suppliedPointer`
    GLuint numBytes() const;
    
    ///
    OpenGLDataBufferMetaData();
    ///
    OpenGLDataBufferMetaData(void * pointer, GLuint bytesPerElement, GLuint numElements);
};

/// Represents a wrapper around data buffer objects in opengl. These are
/// used send data to the GPU for use in your shaders. It can take any type,
/// including Eigen types. All that is requires is that the objects have a
/// default constructor. Data buffer objects come in two flavors:
/// GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER. You are recommended to use
/// the arrayBuffer(...) and elementArrayBuffer(..) static methods to create
/// these two kinds of data buffer types.
class OpenGLDataBuffer : public OpenGLObject {
public:
    
    /// The type buffer, either GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER.
    GLenum type;
    
    ///
    const OpenGLDataBufferMetaData & metaData() const;
    ///
    void setMetaData(const OpenGLDataBufferMetaData & metaData);
    
    /// This constructor copies data of an STL vector
    OpenGLDataBuffer(GLenum type = 0, OpenGLDataBufferMetaData metaData = OpenGLDataBufferMetaData());
    
    /// Creates a GL_ARRAY_BUFFER data buffer object with the given
    /// (optional) `data`.
    static std::shared_ptr<OpenGLDataBuffer> arrayBuffer(const OpenGLDataBufferMetaData & metaData);
    /// Creates a GL_ELEMENT_ARRAY_BUFFER data buffer object with the given
    /// (optional) `metaData`.
    static std::shared_ptr<OpenGLDataBuffer> elementArrayBuffer(const OpenGLDataBufferMetaData & metaData);
    
    /// Makes this the current buffer in opengl and returns the last bound buffer
    /// of this `type`, `0` otherwise.
    GLint setAsActiveDBO();
    /// Returns the buffer binding for `self.type` to `oldBinding`
    void restoreActiveDBO(GLint oldBinding);
    
    /// Send the data of this buffer if needed. `usageType` is one of
    /// either GL_DYNAMIC_DRAW or GL_STATIC_DRAW. If you plan to update this
    /// buffer a lot, use GL_DYNAMIC_DRAW. Otherwise use GL_STATIC_DRAW.
    void sendData(GLenum usageType);
    
    /// Call this to notify this buffer if the data needs to be re-sent to OpenGL
    void setNeedsUpdate();
   
protected:
    /// Override this to actuall allocated content
    virtual void allocateContent();
    /// Override this to actually free the content
    virtual void freeContent();
    
private:
    /// The data of this data buffer object.
    OpenGLDataBufferMetaData metaData_;

    /// Whther or not the data of this buffer has been sent.
    bool dataIsSent_;
    
};

/// Represents an opengl Vertex Array Buffer object. It is used to collect
/// together various DataBufferObjects together for drawing. Note that this
/// is required by OpenGL 3.2+.
class OpenGLVertexArray : public OpenGLObject {
public:
    /// Binds this VAO and then returns the last bound VAO
    GLuint setAsActiveVAO();
    /// Returns the buffer binding for `self.type` to `oldBinding`
    void restoreActiveVAO(GLuint oldBinding);
    
protected:
    /// Override this to actuall allocated content
    virtual void allocateContent();
    /// Override this to actually free the content
    virtual void freeContent();
    
};

///
struct OpenGLTextureMetaData {
    
    /// The type of texture, e.g. GL_TEXTURE_2D. 
    /// Equivalent to the `target` field of a glTextureImage* function
    GLenum targetType;
    /// Specifies the n-th mimmap level of detail, where n=0 is the base texture.
    GLint mipmapLevel;
    
    ///
    bool is1DTexture() const;
    ///
    bool is2DTexture() const;
    ///
    bool is3DTexture() const;
    
    ///
    GLint textureDimension() const;

    /// Specifies the number of color components in the texture. 
    /// See the 3 tables in https://www.opengl.org/sdk/docs/man/html/glTexImage3D.xhtml
    /// for more information. 
    /// Equivalent to the `internalFormat` field of a glTextureImage* function
    GLint internalDataFormat;
    
    /// Specifies the width of the texture
    GLsizei width;
    /// Specifies the height of the texture
    GLsizei height;
    /// Speiifies the depth of the texture
    GLsizei depth;
    
    /// This value must be 0?
    GLint border;
    
    /// The color space of this texture, e.g. GL_RGBA.
    /// Equivalent to the `format` field of a glTextureImage* function
    GLenum pixelFormat;
    /// Specifies the data of the pixel data on a per-pixel level. 
    /// i.e. GL_UNSIGNED_BYTE
    /// Equivalent to the `type` field of a glTextureImage* function
    GLenum pixelType;
    
    /// A pointer to the data of this texture. Use `dataPointer` for accessing
    /// this pointer, if present.
    void * dataPointer;
    
    /// Used to indicated which type of interpolation is used. In general
    /// a bitmap does no linear interpolation, instead favoring pixelated
    /// representations. If `isBitMap` is FALSE, the texture will be smoothed
    /// out by opengl upon render.
    bool bitMapped;
    
    ///
    bool mipMapped;
    ///
    GLfloat mipMapBorderColor[4];
    
    ///
    bool linearlyInterpolated;
    
    ///
    OpenGLTextureMetaData() : targetType(0), mipmapLevel(0), internalDataFormat(0), border(0), pixelFormat(0), pixelType(0), dataPointer(nullptr), bitMapped(false), mipMapped(false), mipMapBorderColor {0, 0, 0, 1}, linearlyInterpolated(false) {}
};

/// Represents a wrapper around an opengl texture. It contains an associated
/// `image` used for storing its pixel data. Note that changed the data
/// in `image` will automaitcally refresh this buffer object once
/// `glSendData()` has been called again. For more information of how to
/// attach a texture to a program, see nsgl::Program.attach(...). These
/// textures are hardcoded to be 2D textures (sorry if you needed something
/// else).
class OpenGLTextureBuffer : OpenGLObject {
public:
    
    ///
    OpenGLTextureBuffer();
    ///
    OpenGLTextureBuffer(GLint textureUnit, const OpenGLTextureMetaData & metaData);
    
    /// Which texture unit this texture will be sent to. Equivalent to
    /// `textureUnit_` - GL_TEXTURE0.
    GLint textureUnit() const;
    ///
    void setTextureUnit(GLint unit);
    
    ///
    const OpenGLTextureMetaData & metaData() const;
    ///
    void setMetaData(const OpenGLTextureMetaData & metaData);
    
    ///
    GLenum glSetActiveTextureUnit();
    
    struct GLState {
        GLenum lastTextureUnit;
        GLuint lastBoundTexture;
    };
    
    /// Makes this the currently active texture in opengl.
    GLState glBind();
    
    /// Unbinds the current texture, and then activates `lastTextureUnit`
    void glUnbind(const GLState & state);
    
    /// Sends the image data of this texture if needed. In particular, this
    /// method boths checks if the image has changed or whether the data
    /// for this texture has never been sent.
    void sendData(bool deleteDataAfterSend = false);
    
    /// Call this to notify this buffer if the data needs to be re-sent to OpenGL
    void setNeedsUpdate();
    
    /// Generates the mip-map (multiple image layers) to speedup texture
    /// sampling when the texture is far away or close up.
    void generateMipMap();
    
    /// Sets the linear interpolation of thie texture. If `isBitMap` is TRUE
    /// then texture will not be anti-aliased.
    void setLinearInterpolation();
  
protected:
    /// Override this to actuall allocated content
    virtual void allocateContent();
    /// Override this to actually free the content
    virtual void freeContent();

private:
    /// The metadata of this texture.
    OpenGLTextureMetaData metaData_;

    /// Which texture unit this texture belongs too. e.g. GL_TEXTURE0.
    GLenum textureUnit_;

    /// Whether or not this texture's image data has been sent.
    bool dataIsSent_;
    
};

#endif /* OpenGLDataBuffer_hpp */
