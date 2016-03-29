///
/// Shader.cpp
/// ----------
/// Nikolai Shkurkin
/// NSGL Library
///

#include "Shader.h"
#include <src/util/parsing/fileIO.h>

/// nsgl extensions for Shaders
void nsgl::setShaderSource(GLuint shaderHandle, GLstring string, GLint * length) {
    const GLchar **constStr = const_cast<const GLchar **>(string);
    glShaderSource(shaderHandle, 1, constStr, length);
}

void nsgl::setShaderSource(GLuint shaderHandle, std::string string) {
    GLint stringLen = string.length();
    const GLchar * str = string.c_str();
    nsgl::setShaderSource(shaderHandle, (GLstring) &str, &stringLen);
}

std::string nsgl::getShaderLog(GLuint shaderHandle) {
    GLsizei outputLength = GLsizei(256);
    GLchar buffer[256] = { 0 };

    glGetShaderInfoLog(shaderHandle, GLsizei(256), &outputLength, buffer);
    return std::string(buffer);
}

void nsgl::printShaderLog(GLuint shaderHandle) {
    std::string toPrint = nsgl::getShaderLog(shaderHandle);
    if (!toPrint.empty()) {
        std::cout << "Shader Log:" << toPrint << "\n";
    }
}

using namespace nsgl;

/// Shader implementation
std::map< std::string, Shader > Shader::shaderCache;

std::string Shader::description() {
    return "(Shader) {\n  fileName: " + fileName + "\n  source: \n"
    + source + "\n}";
}

Shader & Shader::fromResourceFile(GLenum type, std::string fileName) {
    if (shaderCache.count(fileName) == 0) {
        std::string rscName = fileName;
        std::string src = util::textFileRead(rscName);
        
        shaderCache[fileName] = Shader(type, fileName, src);
    }
    return shaderCache[fileName];
}

Shader::Shader() {
    init(0, "", "");
}

Shader::Shader(GLenum type, std::string fileName, std::string source) {
    init(type, fileName, source);
}

void Shader::init(GLenum type, std::string fileName, std::string source) {
    this->fileName = fileName;
    this->type = type;
    this->source = source;
    
    allocated = compiled = false;
    handle = 0;
    compileStatus = GLint(-1);
}

void Shader::glAlloc() {
    if (!allocated) {
        handle = glCreateShader(type);
        allocated = true;
    }
}

bool Shader::glCompile() {
    if (!compiled) {
        glAlloc();
        
        // Set source and compile
        nsgl::setShaderSource(handle, source);
        glCompileShader(handle);
        
        // Show an errors
        nsgl::printStateErrors();
        glGetShaderiv(handle, GLenum(GL_COMPILE_STATUS), &compileStatus);
        nsgl::printShaderLog(handle);
        
        compiled = compileStatus != 0;
    }
    
    return compiled;
}

void Shader::glFree() {
    if (allocated) {
        glDeleteShader(handle);
        handle = 0;
        allocated = false;
        compiled = false;
    }
}
