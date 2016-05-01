//
//  TextureRenderTarget.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/6/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TextureRenderTarget_hpp
#define TextureRenderTarget_hpp

#include <memory>
#include <vector>

#include "OpenGLDataBuffer.hpp"
#include "OpenGLShaders.hpp"

#include "stl_extensions.hpp"

struct TextureRenderTarget {
    std::shared_ptr<OpenGLTextureBuffer> swapTexture;
    std::shared_ptr<OpenGLTextureBuffer> outputTexture;
    std::shared_ptr<OpenGLProgram> program;
    
    std::vector<GLfloat> points;
    std::vector<GLfloat> texcoords;
    
    std::shared_ptr<OpenGLVertexArray> triangleVAO;
    std::shared_ptr<OpenGLDataBuffer> positionDBO;
    std::shared_ptr<OpenGLDataBuffer> texcoordDBO;
    
    bool firstDraw;
    
    /// Call this only when you have a valid OpenGL context available.
    void init(int texWidth, int texHeight, void * texData);
    /// Call only within a valid OpenGL context
    void draw();
};

#endif /* TextureRenderTarget_hpp */
