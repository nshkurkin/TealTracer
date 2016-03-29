///
/// Program.cpp
/// -----------
/// Nikolai Shkurkin
/// NSGL Library
///

#include "Program.h"

/// nsgl extensions for working with programs
std::string nsgl::getProgramLog(GLuint programHandle) {
    GLsizei outputLength = GLsizei(256);
    GLchar buffer[256] = { 0 };

    glGetProgramInfoLog(programHandle, GLsizei(256), &outputLength, buffer);
    return std::string(buffer);
}

void nsgl::printProgramLog(GLuint programHandle) {
    std::string toPrint = nsgl::getProgramLog(programHandle);
    if (!toPrint.empty()) {
        std::cout << "Shader Program Log: " << toPrint << "\n";
    }
}

void nsgl::sendUniMat4(Program &prog, const std::string & uniName,
                       const Eigen::Matrix4f & mat4) {
    if (prog.getUniform(uniName).isValid()) {
        glUniformMatrix4fv(prog.getUniform(uniName).location, 1, GL_FALSE,
                           mat4.data());
    }
}

void nsgl::sendUniform(Program &prog, const std::string & uniName,
                       GLfloat val) {
    if (prog.getUniform(uniName).isValid()) {
        glUniform1f(prog.getUniform(uniName).location, val);
    }
}

void nsgl::sendUniMat4v(Program &prog, const std::string & uniName,
                        int numToSend, const Eigen::Matrix4f & firstMat4) {
    if (prog.getUniform(uniName).isValid()) {
        glUniformMatrix4fv(prog.getUniform(uniName).location, numToSend,
                           GL_FALSE, firstMat4.data());
    }
}

using namespace nsgl;

//////
Program::Program() {
    vertexShader = Shader(GL_VERTEX_SHADER, "", "");
    fragmentShader = Shader(GL_FRAGMENT_SHADER, "", "");
    
    linkStatus = -1;
    handle = 0;
    ignoreWarnings = built = allocated = false;
}

Program::Program(const std::string vShaderName,
                 const std::string fShaderName) {
    linkStatus = -1;
    handle = 0;
    ignoreWarnings = built = allocated = false;
    
    setVertexShader(vShaderName);
    setFragmentShader(fShaderName);
}

void Program::setVertexShader(const std::string & name) {
    vertexShader = Shader::fromResourceFile(GLenum(GL_VERTEX_SHADER), name);
    built = false;
}
void Program::setFragmentShader(const std::string & name) {
    fragmentShader = Shader::fromResourceFile(GLenum(GL_FRAGMENT_SHADER), name);
    built = false;
}

std::string Program::name() {
    return "(Program) (vert: " + vertexShader.fileName + " + frag: "
    + fragmentShader.fileName + ")";
}

bool Program::hasVariable(std::string varName) {
    return glGetAttribLocation(handle, varName.c_str()) >= 0;
}

void Program::glAlloc() {
    if (!allocated) {
        handle = glCreateProgram();
        allocated = true;
    }
}

bool Program::glBuild() {
    if (!built) {
        if (!vertexShader.glCompile()) {
            nsgl::log("Error compiling vertex shader " + vertexShader);
            return false;
        }
        if (!fragmentShader.glCompile()) {
            nsgl::log("Error compiling fragment shader " + fragmentShader);
            return false;
        }
        
        glAlloc();
        glAttachShader(handle, vertexShader.handle);
        glAttachShader(handle, fragmentShader.handle);
        
        glLinkProgram(handle);
        nsgl::printStateErrors();
        glGetProgramiv(handle, GLenum(GL_LINK_STATUS), &linkStatus);
        nsgl::printProgramLog(handle);
        
        if (linkStatus == 0) {
            nsgl::log("Linking failed.");
            return false;
        }
        
        vertexShader.glFree();
        fragmentShader.glFree();
        built = true;
    }
    
    return true;
}

void Program::buildOrCrash() {
    if (!glBuild()) {
        nsgl::log("Failed to install shader program " + name());
        exit(1);
    }
}

void Program::glBind() {
    buildOrCrash();
    glUseProgram(handle);
}

void Program::glUnbind() {
    glUseProgram(0);
}

void Program::glFree() {
    if (allocated) {
        glDeleteProgram(handle);
        handle = 0;
        allocated = false;
        built = false;
    }
}

Uniform Program::getUniform(std::string uniformName) {
    buildOrCrash();
    
    GLint location = glGetUniformLocation(handle, uniformName.c_str());
    Uniform uni = Uniform(uniformName, location);
    
    if (location < 0 && !ignoreWarnings)
        NSGL_LOG("Warning: " + name() + " does not have uniform " + uniformName);
    
    return uni;
}

VertexAttribute Program::getAttribute(std::string attribName, GLint numEls,
                                      GLenum type) {
    buildOrCrash();
    
    GLint location = glGetAttribLocation(handle, attribName.c_str());
    VertexAttribute var = VertexAttribute(attribName, location, numEls, type);
    
    if (location < 0 && !ignoreWarnings)
        NSGL_LOG("Warning: " + name() + " does not have attribute " + attribName);
    
    return var;
}

VertexAttribute Program::getAttribute(std::string attribName) {
    return getAttribute(attribName, 3, GL_FLOAT);
}

VertexAttribute Program::getFloatAttribute(std::string attribName) {
    return getAttribute(attribName, 1, GL_FLOAT);
}

VertexAttribute Program::getVec2fAttribute(std::string attribName) {
    return getAttribute(attribName, 2, GL_FLOAT);
}

VertexAttribute Program::getVec3fAttribute(std::string attribName) {
    return getAttribute(attribName, 3, GL_FLOAT);
}

VertexAttribute Program::getVec4fAttribute(std::string attribName) {
    return getAttribute(attribName, 4, GL_FLOAT);
}

void Program::glSendUniform(const std::string & name, GLfloat f) {
    nsgl::sendUniform(*this, name, f);
}

void Program::glSendUniform(const std::string & name, const Eigen::Matrix<float, 1, 1> & vector, int numVecs) {
    if (getUniform(name).isValid()) {
        glUniform1fv(getUniform(name).location, numVecs, vector.data());
    }
}
void Program::glSendUniform(const std::string & name,
    const Eigen::Matrix<float, 2, 1> & vector, int numVecs) {
    if (getUniform(name).isValid()) {
        glUniform2fv(getUniform(name).location, numVecs, vector.data());
    }
}
void Program::glSendUniform(const std::string & name,
    const Eigen::Matrix<float, 3, 1> & vector, int numVecs) {
    if (getUniform(name).isValid()) {
        glUniform3fv(getUniform(name).location, numVecs, vector.data());
    }
}
void Program::glSendUniform(const std::string & name,
    const Eigen::Matrix<float, 4, 1> & vector, int numVecs) {
    if (getUniform(name).isValid()) {
        glUniform4fv(getUniform(name).location, numVecs, vector.data());
    }
}

void Program::glSendUniform(const std::string & name, util::Mat4f & mat,
                            int numMats) {
    nsgl::sendUniMat4v(*this, name, numMats, mat);
}

void Program::glSendUniform(const std::string & name, int i) {
    glUniform1i(getUniform(name).location, i);
}
