//
//  OpenGLShaders.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/31/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "OpenGLShaders.hpp"

///
std::shared_ptr<GLuint>
OpenGLShaderObject::allocateContent() {
    return std::shared_ptr<GLuint>(new GLuint(glCreateShader(type)));
}

///
void
OpenGLShaderObject::freeContent() {
    glDeleteShader(handle());
}

///
bool
OpenGLShaderVariable::isArray() const {
    return arraySize > 1;
}

///
bool
OpenGLShaderVariable::valid() const {
    return location >= 0;
}

///
OpenGLShaderVariable::OpenGLShaderVariable() {
    init("", Attribute, GLint(-1), GLint(0), GLint(0), GLenum(GL_NONE));
}

///
OpenGLShaderVariable::OpenGLShaderVariable(const std::string & name, Type variableType) {
    init(name, variableType, GLint(-1), GLint(0), GLint(0), GLenum(GL_NONE));
}

///
OpenGLShaderVariable::OpenGLShaderVariable(const std::string & name, Type variableType, GLint location, GLint arraySize, GLint numElements, GLenum valueType) {
    init(name, variableType, location, arraySize, numElements, valueType);
}

///
void
OpenGLShaderVariable::init(const std::string & name, Type variableType, GLint location, GLint arraySize, GLint numElements, GLenum valueType) {
    this->name = name;
    this->variableType = variableType;
    this->location = location;
    this->numElements = numElements;
    this->valueType = valueType;
    this->arraySize = arraySize;
}

///
void
OpenGLShaderVariable::glEnableAttribute() const {
    assert(variableType == Attribute);
    if (valid()) {
        glEnableVertexAttribArray(GLuint(location));
    }
}

///
void
OpenGLShaderVariable::glDisableAttribute() const {
    assert(variableType == Attribute);
    if (valid()) {
        glDisableVertexAttribArray(GLuint(location));
    }
}
///
void
OpenGLShaderVariable::glSetAttributeLayout() const {
    assert(variableType == Attribute);
    if (valid()) {
        glVertexAttribPointer(GLuint(location), numElements, valueType, GLboolean(GL_FALSE), GLsizei(0), nullptr);
    }
}

///
bool
OpenGLShader::isVertexShader() const {
    return type == GLenum(GL_VERTEX_SHADER);
}
///
bool
OpenGLShader::isFragmentShader() const {
    return type == GLenum(GL_FRAGMENT_SHADER);
}
///
bool
OpenGLShader::isGeometryShader() const {
    return type == GLenum(GL_GEOMETRY_SHADER);
}

///
bool
OpenGLShader::compiledSuccessfully() const {
    GLint status;
    glGetShaderiv(handle(), GLenum(GL_COMPILE_STATUS), &status);
    return status == GL_TRUE;
}

///
OpenGLShader::OpenGLShader() {
    init(GLenum(GL_VERTEX_SHADER), "", "");
}

///
OpenGLShader::OpenGLShader(GLenum type, const std::string & source) {
    init(type, source, "");
}

///
OpenGLShader::OpenGLShader(GLenum type, const std::string & source, const std::string & filePath) {
    init(type, source, filePath);
}

///
void
OpenGLShader::init(GLenum type, const std::string & source, const std::string & filePath) {
    this->type = type;
    this->source = source;
    this->filePath = filePath;
}

///
static inline std::string
readFileContent(const std::string & filePath) {
    std::ifstream source;
    source.open(filePath.c_str(), std::ios_base::in);
    if (!source) {
        assert(false);
    }
    
    std::string content;
    std::string line;
    while (!std::getline(source, line).eof()) {
        /// Remove all comments
        content.append(line);
    }
    
    return content;
}

///
OpenGLShader
OpenGLShader::vertexShaderWithSource(const std::string & source) {
    return OpenGLShader(GLenum(GL_VERTEX_SHADER), source);
}

///
OpenGLShader
OpenGLShader::fragmentShaderWithSource(const std::string & source) {
    return OpenGLShader(GLenum(GL_FRAGMENT_SHADER), source);
}

///
OpenGLShader
OpenGLShader::geometryShaderWithSource(const std::string & source) {
    return OpenGLShader(GLenum(GL_GEOMETRY_SHADER), source);
}

///
OpenGLShader
OpenGLShader::vertexShaderWithFilePath(const std::string & filePath) {
    return OpenGLShader(GLenum(GL_VERTEX_SHADER), readFileContent(filePath), filePath);
}

///
OpenGLShader
OpenGLShader::fragmentShaderWithFilePath(const std::string & filePath) {
    return OpenGLShader(GLenum(GL_FRAGMENT_SHADER), readFileContent(filePath), filePath);
}

///
OpenGLShader
OpenGLShader::geometryShaderWithFilePath(const std::string & filePath) {
    return OpenGLShader(GLenum(GL_GEOMETRY_SHADER), readFileContent(filePath), filePath);
}

///
void
OpenGLShader::setupHandleObject(std::shared_ptr<OpenGLShaderObject> object) {
   object->type = type;
}

///
void
OpenGLShader::compile() {
    glAllocate();
    
    const char * lines[1] = { source.c_str() };
    GLint length = (GLint) source.length();
    
    glShaderSource(handle(), GLsizei(1), lines, &length);
    glCompileShader(handle());
    
    if (!compiledSuccessfully()) {
        std::cerr << "{" << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")} Failed to compile shader with source: " << source << " \n\nStatus error: " << statusMessage() << std::endl;
    }
}

///
std::string
OpenGLShader::statusMessage() const {
    GLint logLength, outputLength;
    glGetShaderiv(handle(), GL_INFO_LOG_LENGTH, &logLength);
    char * buffer = new char[logLength + 1];
    glGetShaderInfoLog(handle(), logLength, &outputLength, buffer);
    std::string message(buffer);
    delete[] buffer;
    return message;
}


///
GLboolean
OpenGLProgram::linkSuccess() const {
    GLint status;
    glGetProgramiv(handle(), GLenum(GL_LINK_STATUS), &status);
    return GLboolean(status);
}

///
GLboolean
OpenGLProgram::validationSuccess() const {
    GLint status;
    glGetProgramiv(handle(), GLenum(GL_VALIDATE_STATUS), &status);
    return GLboolean(status);
}

///
std::shared_ptr<GLuint>
OpenGLProgramObject::allocateContent() {
    return std::shared_ptr<GLuint>(new GLuint(glCreateProgram()));
}

///
void
OpenGLProgramObject::freeContent() {
    glDeleteProgram(handle());
}

///
void
OpenGLProgram::build(bool crashOnFailure) {
    glAllocate();

    bool foundBadShader = false;
    int whichShader = 0;
    while (whichShader < shaders.size() && !foundBadShader) {
        auto shader = shaders[whichShader];
        shader.compile();
        foundBadShader = !shader.compiledSuccessfully();
        whichShader++;
    }
    
    if (foundBadShader) {
        std::cerr << "{" << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")} Shader " << (whichShader - 1) << " with source " << shaders[whichShader-1].source << " failed to compile with error log: " << shaders[whichShader-1].statusMessage() << std::endl;
        assert(!crashOnFailure);
    }
    else {
        for (auto itr = shaders.begin(); itr != shaders.end(); itr++) {
            glAttachShader(handle(), (*itr).handle());
        }
        
        glLinkProgram(handle());
        if (linkSuccess()) {
            std::cerr << "{" << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")} Linking failed! OpenGL error: " << statusMessage() << std::endl;
            assert(!crashOnFailure);
        }
        
        gatherAdditionalProgramInfo();
        
        for (auto itr = shaders.begin(); itr != shaders.end(); itr++) {
            (*itr).glFree();
        }
    }

}

///
GLuint
OpenGLProgram::setAsActiveProgram() {
    GLint oldProgram;
    glGetIntegerv(GLenum(GL_CURRENT_PROGRAM), &oldProgram);
    glUseProgram(handle());
    
    return GLuint(oldProgram);
}

///
void
OpenGLProgram::restoreActiveProgram(GLuint oldProgram) const {
    glUseProgram(oldProgram);
}

///
GLint
OpenGLProgram::attachedShaderCount() const {
    GLint count;
    glGetProgramiv(handle(), GLenum(GL_ATTACHED_SHADERS), &count);
    return count;
}

///
GLint
OpenGLProgram::activeAttributeCount() const {
    GLint count;
    glGetProgramiv(handle(), GLenum(GL_ACTIVE_ATTRIBUTES), &count);
    return count;
}

///
GLint
OpenGLProgram::activeUniformCount() const {
    GLint count;
    glGetProgramiv(handle(), GLenum(GL_ACTIVE_UNIFORMS), &count);
    return count;
}

/// Returns the type information for the OpenGL type `type`. Type info with a
/// name == "<unknown>" is not recognized by this function.
static inline OpenGLTypeInfo getTypeInfoForGLType(GLenum type) {
    static std::map<GLenum, OpenGLTypeInfo> info;

    if (info.empty() == 0) {
        #define setField(type, name, size, els, elType) \
            info[GLenum(type)] = OpenGLTypeInfo(GLenum(type), name, GLsizei(size), GLuint(els), GLenum(elType))
        
        setField(GL_BYTE, "GLbyte", sizeof(GLbyte), 1, GL_BYTE);
        setField(GL_UNSIGNED_BYTE, "GLubyte", sizeof(GLubyte), 1, GL_UNSIGNED_BYTE);
        setField(GL_SHORT, "GLshort", sizeof(GLshort), 1, GL_SHORT);
        setField(GL_UNSIGNED_SHORT, "GLushort", sizeof(GLushort), 1, GL_UNSIGNED_SHORT);
        setField(GL_INT, "GLint", sizeof(GLint), 1, GL_INT);
        setField(GL_UNSIGNED_INT, "GLuint", sizeof(GLuint), 1, GL_UNSIGNED_INT);
        setField(GL_FLOAT, "GLfloat", sizeof(GLfloat), 1, GL_FLOAT);
        setField(GL_BOOL, "GLboolean", sizeof(GLboolean), 1, GL_BOOL);
        setField(GL_FLOAT_VEC2, "GLfloat[2]", 2 * sizeof(GLfloat), 2, GL_FLOAT);
        setField(GL_FLOAT_VEC3, "GLfloat[3]", 3 * sizeof(GLfloat), 3, GL_FLOAT);
        setField(GL_FLOAT_VEC4, "GLfloat[4]", 4 * sizeof(GLfloat), 4, GL_FLOAT);
        setField(GL_FLOAT_MAT2, "GLfloat[2][2]", 2 * 2 * sizeof(GLfloat), 4, GL_FLOAT);
        setField(GL_FLOAT_MAT3, "GLfloat[3][3]", 3 * 3 * sizeof(GLfloat), 9, GL_FLOAT);
        setField(GL_FLOAT_MAT4, "GLfloat[4][4]", 4 * 4 * sizeof(GLfloat), 16, GL_FLOAT);
        setField(GL_SAMPLER_2D, "<sampler 2D>", 0, 0, GL_SAMPLER_2D);
        setField(GL_SAMPLER_3D, "<sampler 3D>", 0, 0, GL_SAMPLER_3D);
        setField(GL_SAMPLER_CUBE, "<sampler cube>", 0, 0, GL_SAMPLER_CUBE);
        setField(GL_SAMPLER_2D_SHADOW, "<sampler 2D shadow>", 0, 0, GL_SAMPLER_2D_SHADOW);
    }
    
    if (info.count(type) != 0) {
        return info[type];
    }
    else {
        return OpenGLTypeInfo(type, "<unknown>", GLsizei(0), GLuint(0), type);
    }
}

///
void
OpenGLProgram::gatherAdditionalProgramInfo() {
    
    activeAttributeMap.clear();
    activeUniformMap.clear();
    
    GLint size;
    GLenum type;
    char * name;
    GLint nameLength;
    GLsizei actualNameLength;
    
    for (int attribIdx = 0; attribIdx < activeAttributeCount(); attribIdx++) {
        
        glGetProgramiv(handle(), GLenum(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH), &nameLength);
        name = new char[nameLength];
        
        glGetActiveAttrib(handle(), attribIdx, GLsizei(nameLength), &actualNameLength, &size, &type, name);
        
        auto typeInfo = getTypeInfoForGLType(type);
        std::string attribName(name);
        auto attribute = OpenGLShaderVariable(attribName, OpenGLShaderVariable::Attribute, glGetAttribLocation(handle(), attribName.c_str()), size, GLint(typeInfo.numElements), typeInfo.elementType);
        
        activeAttributeMap[attribName] = attribute;
        
        delete[] name;
    }
    
    for (int uniformIdx = 0; uniformIdx < activeUniformCount(); uniformIdx++) {
        const GLuint idx = uniformIdx;

        glGetActiveUniformsiv(handle(), GLsizei(1), &idx, GLenum(GL_UNIFORM_NAME_LENGTH), &nameLength);
        name = new char[nameLength];
        
        glGetActiveUniform(handle(), uniformIdx, GLsizei(nameLength), &actualNameLength, &size, &type, name);
        
        auto typeInfo = getTypeInfoForGLType(type);
        std::string uniformName(name);
        auto uniform = OpenGLShaderVariable(uniformName, OpenGLShaderVariable::Uniform, glGetUniformLocation(handle(), uniformName.c_str()), size, GLint(typeInfo.numElements), typeInfo.elementType);
        
        activeUniformMap[uniformName] = uniform;
        
        delete[] name;
    }
}

///
std::string
OpenGLProgram::statusMessage() const {
    GLint logLength, outputLength;
    glGetProgramiv(handle(), GL_INFO_LOG_LENGTH, &logLength);
    char * buffer = new char[logLength + 1];
    glGetProgramInfoLog(handle(), logLength, &outputLength, buffer);
    std::string message(buffer);
    delete[] buffer;
    return message;
}

