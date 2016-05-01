//
//  TextureRenderTarget.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/6/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TextureRenderTarget.hpp"

///
void TextureRenderTarget::init(int texWidth, int texHeight, void * texData) {
    firstDraw = false;
    
    this->points = make_vector<GLfloat>(
        -1.0, -1.0, 0.0,
         1.0, -1.0, 0.0,
         1.0,  1.0, 0.0,
         
        -1.0, -1.0, 0.0,
         1.0,  1.0, 0.0,
        -1.0,  1.0, 0.0
    );
    this->texcoords = make_vector<GLfloat>(
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        
        0.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    );
        
    ///
    this->positionDBO = OpenGLDataBuffer::arrayBuffer(OpenGLDataBufferMetaData(&this->points[0], GLuint(sizeof(GLfloat) * 3), GLuint(this->points.size())));
    this->positionDBO->sendData(GLenum(GL_STATIC_DRAW));
    
    this->texcoordDBO = OpenGLDataBuffer::arrayBuffer(OpenGLDataBufferMetaData(&this->texcoords[0], GLuint(sizeof(GLfloat) * 2), GLuint(this->texcoords.size())));
    this->texcoordDBO->sendData(GLenum(GL_STATIC_DRAW));

    this->program = std::shared_ptr<OpenGLProgram>(new OpenGLProgram());
    this->program->shaders = make_vector<std::shared_ptr<OpenGLShader>>(
        OpenGLShader::vertexShaderWithFilePath("texture.vert.glsl"),
        OpenGLShader::fragmentShaderWithFilePath("texture.frag.glsl")
    );
    
    this->triangleVAO = std::shared_ptr<OpenGLVertexArray>(new OpenGLVertexArray());
    
    this->program->build(true);
    this->program->connectDataToProgram(this->triangleVAO.get(), make_map<std::string, OpenGLDataBuffer*>(
        "vertex_position", this->positionDBO.get(),
        "vertex_texcoord", this->texcoordDBO.get()
    ));

    auto textureFormat = OpenGLTextureMetaData();
    
    textureFormat.targetType = GL_TEXTURE_2D;
    textureFormat.pixelFormat = GL_RGBA;
    textureFormat.pixelType = GL_UNSIGNED_BYTE;
    textureFormat.internalDataFormat = GL_RGBA;
    textureFormat.width = texWidth;
    textureFormat.height = texHeight;
    textureFormat.linearlyInterpolated = true;
    textureFormat.mipMapped = true;
    textureFormat.dataPointer = texData;
    
    this->outputTexture = std::shared_ptr<OpenGLTextureBuffer>(new OpenGLTextureBuffer(0, textureFormat));
    this->outputTexture->sendData();
    
    this->swapTexture = std::shared_ptr<OpenGLTextureBuffer>(new OpenGLTextureBuffer(0, textureFormat));
    this->swapTexture->sendData();
}

///
void TextureRenderTarget::draw() {
    auto lastProgramHandle = this->program->setAsActiveProgram();
    OpenGLTextureBuffer::GLState oldTextureState;
    
    auto texture = this->outputTexture;
    
    if (!firstDraw) {
        texture->sendData();
        oldTextureState = texture->glBind();
        this->program->attach("tex", texture.get());
    }
    
    auto oldVaoHandle = this->triangleVAO->setAsActiveVAO();
    glDrawArrays(GLenum(GL_TRIANGLES), 0, GLsizei(this->points.size()));
    
    if (!firstDraw) {
        texture->glUnbind(oldTextureState);
    }
    this->triangleVAO->restoreActiveVAO(oldVaoHandle);
    this->program->restoreActiveProgram(lastProgramHandle);
    firstDraw = false;
}
