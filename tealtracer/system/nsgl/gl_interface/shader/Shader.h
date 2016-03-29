///
/// Shader.h
/// --------
/// Nikolai Shkurkin
/// NSGL Library
///


#ifndef ____nsgl_Shader__
#define ____nsgl_Shader__

#include <string>
#include <map>

#include <src/nsgl/nsgl_base.h>
//#include "../../nsgl_base.h"

namespace nsgl {
    /// Sets the shader source for `shaderHandle` using `string` of length
    /// pointed to by `length`.
    void setShaderSource(GLuint shaderHandle, GLstring string, GLint * length);
    /// Sets the shader source for `shaderHandle` using `string`.
    void setShaderSource(GLuint shaderHandle, std::string string);
    /// Gets the shader log for `shaderHandle`.
    std::string getShaderLog(GLuint shaderHandle);
    /// Prints the shader log to stdout for `shaderHandle`.
    void printShaderLog(GLuint shaderHandle);
}

namespace nsgl {
    
    /// Represents a vertex or fragment shader. You typically do not need to use
    /// a Shader directly, but instead use `Program`.
    struct Shader {
    public:
        /// The gl handle for this shader.
        GLuint handle;
        /// The status of compilation.
        GLint compileStatus;
        /// The type of shader. Either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
        GLenum type;
        /// The file name of this shader.
        std::string fileName;
        /// The raw source of this shader.
        std::string source;
        /// Whether this shader has been allocated in opengl.
        bool allocated;
        /// Whether this shader has been compiled in opengl.
        bool compiled;
        
        /// A description of this shader.
        std::string description();
        
        /// Represents a global shader cache of all loaded shaders. Shaders are
        /// access by their file names.
        static std::map< std::string, Shader > shaderCache;
        /// Loads a shader of type `type` from a file name `fileName`.
        static Shader & fromResourceFile(GLenum type, std::string fileName);
        
        /// Creates an empty shader.
        Shader();
        /// Create a shader with the given fileName, source, and type.
        Shader(GLenum type, std::string fileName, std::string source);
        /// In-place initializer for a Shader.
        void init(GLenum type, std::string fileName, std::string source);
        
        /// Wrapper around setting up a vertex shader.
        static Shader vertexShader(std::string fileName, std::string source) {
            return Shader(GL_VERTEX_SHADER, fileName, source);
        }
        /// Wrapper around setting up a fragment shader.
        static Shader fragmentShader(std::string fileName, std::string source) {
            return Shader(GL_FRAGMENT_SHADER, fileName, source);
        }
        
        /// Allocates an opengl object, making `handle` point to a valid shader.
        void glAlloc();
        /// Compiles this shader, setting `compileStatus`.
        bool glCompile();
        /// Frees all associated data in opengl about this shader.
        void glFree();
    };
    
}

#endif 
