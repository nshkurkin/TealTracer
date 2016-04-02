//
//  OpenGLShaders.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/31/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef OpenGLShaders_hpp
#define OpenGLShaders_hpp

#include "gl_include.h"
#include "OpenGLObject.hpp"

#include <map>
#include <string>
#include <vector>
#include <memory>

#include <iostream>
#include <fstream>

#include <cassert>

/// Represents a connection to a shader program's variable. Variables include
/// all of the types listed under `OpenGLShaderVariable.Type`. Typically you
/// don't create one of these manually and instead get them from the
//// `OpenGLProgram` instance that has already been compiled.
///
/// `Attribute`s are per-vertex data sent to shaders like "positions",
/// "normals", etc.. One can set attribute layouts and to enable a particular
/// attribute.
///
/// `Uniform`s constant data in an OpenGL shader program.
///
/// `Unspecified` is used to signify that this variable is likely just a
/// placeholder until its information is manually filled in by the programmer
/// and not by OpenGLProgram.
class OpenGLShaderVariable {
public:
    /// Represents the type of shader variable
    enum Type {
        /// Not yet determined, meant to be a temporary placeholder.
        Unspecified = 0,
        /// Vertex attributes
        Attribute,
        /// Shader Uniforms
        Uniform
    };

    /// The name of this attribute in the shader.
    std::string name;
    /// The handle to the shader's attribute location.
    GLint location;
    /// The number of elements this attribute has (e.g. 3 for a vec3).
    GLint numElements;
    /// The type of this attribute. e.g. GL_FLOAT.
    GLenum valueType;
    /// The size of the array this variable encapsulates, Typically this is only 1
    GLint arraySize;
    
    /// The type of the variable (Attribute, Uniform, etc.)
    Type variableType;
    
    /// Whether or not `name[i]` is a sub-element of this shader variable
    bool isArray() const;
    
    /// Whether `location` is invalid.
    bool valid() const;
    
    ///
    OpenGLShaderVariable();
    /// Creates a named sttribute.
    OpenGLShaderVariable(const std::string & name, Type variableType = Unspecified);
    /// Create an attribute linked to a shader.
    OpenGLShaderVariable(const std::string & name, Type variableType, GLint location, GLint arraySize, GLint numElements, GLenum valueType);
    
    /// In-place initialization of this object
    void init(const std::string & name, Type variableType, GLint location, GLint arraySize, GLint numElements, GLenum valueType);
    
    /// For Attributes only: Enables the attribute given at `location`.
    void glEnableAttribute() const;
    /// For Attributes only: Disable the attribute given at `location`.
    void glDisableAttribute() const;
    
    /// For Attributes only: Sets the pointer layout of `location`. For more 
    /// advanced layout settings you should do it manually using opengl.
    void glSetAttributeLayout() const;
    
};

///
class OpenGLShaderObject : public OpenGLObject {
protected:
    friend class OpenGLShader;
    /// The type of this shader, one of GL_*_SHADER
    GLenum type;

protected:
    /// Override this to actuall allocated content
    virtual std::shared_ptr<GLuint> allocateContent();
    /// Override this to actually free the content
    virtual void freeContent();
};

/// Represents a Shader, which is a component of a Program. The principle operation to perform on
/// a shader is compiling the shader and then checking the compilation status.
class OpenGLShader : public OpenGLObjectManager<OpenGLShaderObject> {
public:
    /// The type of this shader, one of GL_*_SHADER
    GLenum type;
    /// Whether this shader is a vertex shader
    bool isVertexShader() const;
    /// Whether this shader is a fragment shader
    bool isFragmentShader() const;
    /// Whether this shader is a geometry shader
    bool isGeometryShader() const;
    
    /// The supplied path to the source of this shader
    std::string filePath;
    /// The supplied source of this shader
    std::string source;
    
    /// Returns the compilation status of this shader. `nil` implies this shader has never been
    /// allocated.
    bool compiledSuccessfully() const;
    
    ///
    OpenGLShader();
    /// Constructs a shader with a given `type` and `source` code.
    OpenGLShader(GLenum type, const std::string & source);
    /// Constructs a shader with a given `type` and `source` code.
    OpenGLShader(GLenum type, const std::string & source, const std::string & filePath);
    
    ///
    void init(GLenum type, const std::string & source, const std::string & filePath);
    
    /// Creates a vertex shader with the given source.
    static OpenGLShader vertexShaderWithSource(const std::string & source);
    /// Creates a fragment shader with the given source.
    static OpenGLShader fragmentShaderWithSource(const std::string & source);
    /// Creates a goemetry shader with the given source.
    static OpenGLShader geometryShaderWithSource(const std::string & source);
    
    /// Tries to create a vertex shader with the given `filePath`.
    static OpenGLShader vertexShaderWithFilePath(const std::string & filePath);
    /// Tries to create a fragment shader with the given `filePath`.
    static OpenGLShader fragmentShaderWithFilePath(const std::string & filePath);
    /// Tries to create a geometry shader with the given `filePath`.
    static OpenGLShader geometryShaderWithFilePath(const std::string & filePath);
    
protected:

    ///
    virtual void setupHandleObject(std::shared_ptr<OpenGLShaderObject> object);
    
public:
    
    /// Compiles this shader if there is a supplied `source` for this shader.
    void compile();
    
    /// The current status message of this shader, representing the result of the info log for 
    /// this shader. Otherwise, if the shader has not been allocated, a message indicating that
    /// is the case.
    std::string statusMessage() const;
};

/// Computed static information that relates OpenGL type information.
struct OpenGLTypeInfo {
    /// The OpenGL type
    GLenum type;
    /// A human-readable string representation of `type`
    std::string name;
    /// The number of bytes of `type`
    GLsizei size;
    /// The number of sub-components of this type. Usually is just 1.
    GLuint numElements;
    /// The type of the sub-components of this type. Usually == type.
    GLenum elementType;
    
    OpenGLTypeInfo() {}
    OpenGLTypeInfo(GLenum type, const std::string & name, GLsizei size, GLuint numElements, GLenum elementType) : type(type), name(name), size(size), numElements(numElements), elementType(elementType) {}
};

class OpenGLProgramObject : public OpenGLObject {
protected:
    /// Override this to actuall allocated content
    virtual std::shared_ptr<GLuint> allocateContent();

    /// Override this to actually free the content
    virtual void freeContent();
};

/// Represents a Program, which is a collection of Shaders. Programs are built and linked against
/// the shaders that make them up. Each shader in a program must uniquely represent a particular 
/// part of the shader pipeline.
///
/// Useful online resource: http://antongerdelan.net/opengl/shaders.html
class OpenGLProgram : public OpenGLObjectManager<OpenGLProgramObject> {
public:
    /// The link status of this program.
    GLboolean linkSuccess() const;
    
    /// The validation status of this program.
    GLboolean validationSuccess() const;
    
    /// The shaders that make up this program.
    std::vector<OpenGLShader> shaders;
    
    /// Builds and links this program, optionally crashing when any particular
    /// step of the process of compilation and linking fails. This function
    /// handles allocating the program, compilation each shader, linking them
    /// together, and then gathering together each of the active uniforms and
    /// attributes in the program.
    void build(bool crashOnFailure = false);
    
    /// Equivalent to glUseProgram(handle).
    GLuint setAsActiveProgram();
    
    /// Used to restore the last active program, typically from the result of `setAsActiveProgram()`.
    void restoreActiveProgram(GLuint oldProgram) const;
    
    /// The number of attached shaders as reported by OpenGL
    GLint attachedShaderCount() const;
    /// The number of active attributes as reported by OpenGL
    GLint activeAttributeCount() const;
    /// The number of active uniforms as reported by OpenGL
    GLint activeUniformCount() const;
    
    /// A mapping of active attributes of this program that maps the name of the attribute to its
    /// reference. This property should only be used after `build()` has been called.
    std::map<std::string, OpenGLShaderVariable> activeAttributeMap;
    /// A mapping of active uniforms of this program that maps the name of the uniform to its
    /// reference. This property should only be used after `build()` has been called.
    std::map<std::string, OpenGLShaderVariable> activeUniformMap;
    
private:
    
    /// To be called after `build()`, this function gathers all of the active uniforms and 
    /// attributes of this program.
    void gatherAdditionalProgramInfo();

public:

    /// The current status message of this program, representing the result of the info log for
    /// this program. Otherwise, if the program has not been allocated, a message indicating that
    /// is the case.
    std::string statusMessage() const;
    
//    /// Connects each attribute name and buffer data pair present in `attribNamesAndBuffers` to the
//    /// supplied `vao` and the active attributs in this program. After this function is called,
//    /// one can simply set `vao` as active to bring back up all the connections created here
//    /// for rendering purposes. 
//    func connectDataToProgram(vao: OpenGLVertexArrayObject, attribNamesAndBuffers: [String:OpenGLDataBufferObject]) {
//        let oldProgramHandle = setAsActiveProgram()
//        let oldVaoHandle = vao.setAsActiveVAO()
//        
//        for (attribName, buffer) in attribNamesAndBuffers {
//            let attrib = activeAttributeMap[attribName]
//            let oldDboHandle = buffer.setAsActiveDBO()
//            attrib?.glSetAttributeLayout()
//            attrib?.glEnableAttribute()
//            buffer.restoreActiveDBO(oldDboHandle)
//        }
//
//        vao.restoreActiveVAO(oldVaoHandle)
//        restoreActiveProgram(oldProgramHandle)
//    }
};



#endif /* OpenGLShaders_hpp */
