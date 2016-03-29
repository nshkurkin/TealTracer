///
/// TextureBufferObject.h
/// ---------------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_TextureBufferObject__
#define ____nsgl_TextureBufferObject__

#include <vector>
#include <string>
#include <Eigen/Dense>

#include <src/util/data/Image.h>
//#include "../../../util/data/Image.h"
#include <src/nsgl/gl_interface/gl.h>

namespace nsgl {
    
    /// Represents a wrapper around an opengl texture. It contains an associated
    /// `image` used for storing its pixel data. Note that changed the data
    /// in `image` will automaitcally refresh this buffer object once
    /// `glSendData()` has been called again. For more information of how to
    /// attach a texture to a program, see nsgl::Program.attach(...). These
    /// textures are hardcoded to be 2D textures (sorry if you needed something
    /// else).
    template< typename PixelType >
    struct TextureBufferObject {
    public:
        typedef typename util::Image<PixelType>::CollectionType CollectionType;
        
        /// The iamge that this texture wraps over.
        util::Image<PixelType> image;
        /// Returns a reference to the raw data of this texture.
        CollectionType & data() {return image.pixels;}
        
        /// Which texture unit this texture belongs too. e.g. GL_TEXTURE0.
        GLenum textureUnit;
        /// The formate of this texture data, e.g. GL_UNSIGNED_INT.
        GLenum textureDataFormat;
        /// The type of texture, e.g. GL_TEXTURE_2D.
        GLenum textureType;
        /// The color space of this texture, e.g. GL_RGBA.
        GLint textureColorspace;
        /// Which texture unit this texture will be sent to. Equivalent to
        /// `textureUnit` - GL_TEXTURE0.
        GLint whichTextureUnit;
        /// The opengl handle.
        GLuint handle;
        
        /// Returns the width of this texture in pixels.
        int width() {return image.width;}
        /// Returns the height of this texture in pixels.
        int height() {return image.height;}
        
        /// Whether or not this texture has been allocated in opengl.
        bool allocated;
        /// Whether or not this texture's image data has been sent.
        bool dataIsSent;
        /// Used to indicated which type of interpolation is used. In general
        /// a bitmap does no linear interpolation, instead favoring pixelated
        /// representations. If `isBitMap` is FALSE, the texture will be smoothed
        /// out by opengl upon render.
        bool isBitMap;
        
        TextureBufferObject(const util::Image<PixelType> & image, GLint colorSpace,
         GLenum dataFormat, GLenum textureUnit) {
            this->init(image, colorSpace, dataFormat, textureUnit);
        }
        
        TextureBufferObject(const TextureBufferObject<PixelType> & other) {
            this->init(other.image, other.textureColorspace,
             other.textureDataFormat, other.textureUnit);
        }
        
        /// In-place initializer for this TBO given a starting image, color
        /// space, data format, and texture unit.
        void init(util::Image<PixelType> img, GLint colorSpace, GLenum dataFormat,
         GLenum textureUnit) {
            
            this->image = img;
            this->textureColorspace = colorSpace;
            this->textureDataFormat = dataFormat;
            this->textureUnit = textureUnit;
            this->whichTextureUnit = GLint(textureUnit - GLenum(GL_TEXTURE0));
            this->textureType = GLenum(GL_TEXTURE_2D);
            
            handle = -1;
            allocated = false;
            dataIsSent = false;
            
            isBitMap = false;
        }
        
        /// Allocates this texture in opengl as needed.
        void glAlloc() {
            if (!allocated) {
                glGenTextures(1, &handle);
                allocated = true;
            }
        }
        
        /// Frees all of the opengl data associated with this texture.
        void glFree() {
            if (allocated) {
                glDeleteTextures(1, &handle);
                handle = -1;
                allocated = false;
                setNeedsUpdate();
            }
        }
        
        /// Makes this the currently active texture in opengl.
        void glBind() {
            glAlloc();
            glActiveTexture(textureUnit);
            glBindTexture(textureType, handle);
        }
        
        /// Sets the ative texture to texture 0.
        void glUnbind() {
            glActiveTexture(textureUnit);
            glBindTexture(textureType, 0);
        }
        
        /// Sends the iamge data of this texture if needed. In particular, this
        /// method boths checks if the image has changed or whether the data
        /// for this texture has never been sent.
        void glSendData() {
            if (!dataIsSent || image.changed) {
                glBind();
                
                if (data().size() == 0) {
                    glTexImage2D(textureType, 0, textureColorspace, 0,
                     0, 0, textureColorspace, textureDataFormat, NULL);
                }
                else {
                    glTexImage2D(textureType, 0, textureColorspace, width(),
                     height(), 0, textureColorspace, textureDataFormat, &data()[0]);
                }
                
                generateMipMap();
                setLinearInterpolation();

                dataIsSent = true;
                image.clearChanged();
                glUnbind();
            }
        }
        
        /// Call this to notify this buffer if the data needs to be re-sent to OpenGL
        void setNeedsUpdate() {
            dataIsSent = false;
        }
        
        /// Generates the mip-map (multiple image layers) to speedup texture
        /// sampling when the texture is far away or close up.
        void generateMipMap() {
            float bgColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            
            glBind();
            // Generate image pyramid
            glGenerateMipmap(textureType);
            // Set texture wrap modes for the S and T directions
            glTexParameterf(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bgColor);
            
            glUnbind();
        }
        
        /// Sets the linear interpolation of thie texture. If `isBitMap` is TRUE
        /// then texture will not be anti-aliased.
        void setLinearInterpolation() {
            glBind();
            
            if (isBitMap) {
                glTexParameterf(textureType, GL_TEXTURE_MAG_FILTER,
                 GL_NEAREST);
                glTexParameterf(textureType, GL_TEXTURE_MIN_FILTER,
                 GL_NEAREST_MIPMAP_NEAREST);
            }
            else {
                glTexParameterf(textureType, GL_TEXTURE_MAG_FILTER,
                 GL_LINEAR);
                glTexParameterf(textureType, GL_TEXTURE_MIN_FILTER,
                 GL_LINEAR_MIPMAP_LINEAR);
            }
            
            glUnbind();
        }
        
        /// Returns whether this texture is valid in opengl.
        bool isValid() {return handle >= 0;}
        
    };
    
}

#endif // ____nsgl_TextureBufferObject__
