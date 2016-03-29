///
/// Program.h
/// ---------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_Program__
#define ____nsgl_Program__

#include <iostream>
#include <string>
#include <map>
#include <Eigen/Dense>

#include <src/nsgl/nsgl_base.h>
//#include "../../nsgl_base.h"

#include "Shader.h"
#include "Uniform.h"
#include "VertexAttribute.h"

#include <src/nsgl/gl_interface/buffer/TextureBufferObject.h>
//#include "../buffer/TextureBufferObject.h"

namespace nsgl {
    /// Returns the log of a program given by `programHandle`.
    std::string getProgramLog(GLuint programHandle);
    /// Prints out the log of the program given by `programHandle`.
    void printProgramLog(GLuint programHandle);
}

namespace nsgl {
    /// Represents an OpenGL program. This is typically the only object you
    /// need to interact with, instead of directly using the `nsgl::Shader` object.
    /// It allows you to quickly load shaders as well as compile a program.
    /// Similarly, it allows you to gain access to attributes via `get*Attribute`
    /// and facilitates sending uniforms via `nsgl::Program.glSendUniform` which
    /// can send floats, ints, nsgl::Vec*f, and nsgl::Mat4f's. Example:
    ///
    /// <pre>
    /// // Here, "vert.glsl" and "frag.glsl" are local files that contain
    /// // the contents of my vertex and framgent shaders respectively.
    /// nsgl::Program * myProg = new nsgl::Program("vert.glsl", "frag.glsl");
    /// // Typically you want your program to exit if your shaders are broken.
    /// // So here you can easily have the program crash if your program fails
    /// // to compile and link ("build").
    /// myProg->buildOrCrash();
    ///
    /// // ... Later ...
    ///
    /// // Before we use our program, we need to bind it to make it the current
    /// // program.
    /// myProg->glBind();
    /// // Now we can send the light position to a phong shader
    /// myProg->glSendUniform("ligh_pos", nsgl::Vec3f(0, 10, 5));
    ///
    /// // ...
    ///
    /// // Connect attributes in a model class
    /// nsgl::VertexAttribute pos = myProg->getVec3fAttribute("obj_position");
    /// nsgl::VecterAttribute nor = myProg->getVec3fAttribute("obj_normal");
    ///
    /// positions.glSendData(GL_STATIC_DRAW);
    /// normals.glSendData(GL_STATIC_DRAW);
    ///
    /// // To connect these attributes to a buffer object and vertex array object,
    /// // we use the following special command:
    /// nsgl::attachDataAttributeArray(vao, positions, pos);
    /// nsgl::attachDataAttributeArray(vao, normals, nor);
    /// // Now the attribute "obj_position" is connected to "positions" in "vao"
    /// // as is the attribute "obj_normal" connected to "normals" in "vao".
    /// </pre>
    struct Program {
    public:
        /// The link status of the program.
        GLint linkStatus;
        /// The gl handle on this program.
        GLuint handle;
        
        Shader vertexShader, fragmentShader;
        /// Whether or not this program has been built.
        bool built;
        /// Whether or not this program has been allocated within opengl.
        bool allocated;
        
        /// Set this to true for the program to ignore access to vairables that
        /// don't exist within the program. This is useful when you're writing
        /// a generic shader where objects that try access it may be requesting
        /// variables that may not exist within the program.
        bool ignoreWarnings;
        
        /// Creates an empty program.
        Program();
        /// Creates a program by loading a vertex shader from a local file named
        /// `vShaderName`, and likewise for a frament shader using `fShaderName`.
        Program(const std::string vShaderName, const std::string fShaderName);
        
        /// Sets the current vertex shader to the file named `name`.
        void setVertexShader(const std::string & name);
        /// Sets the current fragment shader to the file name `name`.
        void setFragmentShader(const std::string & name);
        /// Returns the name of this program. Typically this a composite of the
        /// names of the shaders.
        std::string name();
        
        /// Returns TRUE IFF `varName` is valid within this program.
        bool hasVariable(std::string varName);
        
        /// Allocates this program in opengl.
        void glAlloc();
        /// Tries to build the program, return TRUE IFF the uild was successful.
        /// A "build" is compiling the shaders and then linking them. After this
        /// the `linkStatus` variable will be set.
        bool glBuild();
        /// Tries to build the program but crashes the program if it fails.
        void buildOrCrash();
        
        /// Make sthis program the current one. Make sure to call this so that
        /// draw calls are send to this program.
        void glBind();
        /// Makes program "0" the current program (no program).
        void glUnbind();
        /// Releases any opengl resources associated with this program.
        void glFree();
        
        /// Gets a wrapper around a uniform of the name `uniformName`.
        Uniform getUniform(std::string uniformName);
        
        /// Gets a custom attribute from a program if it exists.
        VertexAttribute getAttribute(std::string attribName,
         GLint numElements, GLenum type);
        /// Gets a Vec3f attribute (default) of name `attribName`.
        VertexAttribute getAttribute(std::string attribName);
        /// Gets a float attribute of name `attribName`.
        VertexAttribute getFloatAttribute(std::string attribName);
        /// Gets a Vec2f attribute of name `attribName`.
        VertexAttribute getVec2fAttribute(std::string attribName);
        /// Gets a Vec3f attribute of name `attribName`.
        VertexAttribute getVec3fAttribute(std::string attribName);
        /// Gets a Vec4f attribute of name `attribName`.
        VertexAttribute getVec4fAttribute(std::string attribName);
        
        /// Sends a float attribute of name `name`.
        void glSendUniform(const std::string & name, GLfloat f);
        /// Sends any manner of a Vector*f to the program.
        //template <int n>
        //void glSendUniform(const std::string & name,
        //                   const Eigen::Matrix<GLfloat, n, 1> & vector, int numVecs = 1);
		void glSendUniform(const std::string & name,
		                   const Eigen::Matrix<float, 1, 1> & vector, int numVecs = 1);
		void glSendUniform(const std::string & name,
		                   const Eigen::Matrix<float, 2, 1> & vector, int numVecs = 1);
        void glSendUniform(const std::string & name,
                           const Eigen::Matrix<float, 3, 1> & vector, int numVecs = 1);
        void glSendUniform(const std::string & name,
                           const Eigen::Matrix<float, 4, 1> & vector, int numVecs = 1);
        /// Sends any number of Mat4s to the program of name `name`.
        void glSendUniform(const std::string & name, util::Mat4f & mat,
                           int numMats = 1);
        /// Sends and int of name `name` to this program.
        void glSendUniform(const std::string & name, int i);
        template <typename T>
        /// "Attaches" a texture named `name` to this program. By "attachment"
        /// it is mean to say we send the texture unit number to the program.
        void attach(const std::string & name, TextureBufferObject<T> & tbo);
    };
    
}

namespace nsgl {
    /// Sends a single Mat4 to a program.
    void sendUniMat4(Program &prog, const std::string & uniName,
                     const util::Mat4f & mat4);
    /// Sends a bunch of Mat4's to a program.
    void sendUniMat4v(Program &prog, const std::string & uniName,
                      int numToSend, const util::Mat4f & firstMat4);
    /// Sends a float to a program.
    void sendUniform(Program &prog, const std::string & uniName,
                     GLfloat val);
    /// Sends any manner of Vec*f to the program.
    /*template <int n>
    void sendUniform(Program &prog, const std::string & uniName,
                     const Eigen::Matrix<GLfloat, n, 1> & vec, int numVecs = 1) {
        typedef void (*UniformFuncType) (GLint, GLsizei, const GLfloat *);
        static UniformFuncType uniFuncs[4] = {
            (UniformFuncType)glUniform1fv,
            (UniformFuncType)glUniform2fv,
            (UniformFuncType)glUniform3fv,
            (UniformFuncType)glUniform4fv
        };
        
        if (prog.getUniform(uniName).isValid() && n <= 4) {
            (uniFuncs[n - 1])(prog.getUniform(uniName).location, numVecs,
                              vec.data());
        }
    }*/
}

namespace nsgl {
    /*template <int n>
    void Program::glSendUniform(const std::string & name,
                                const Eigen::Matrix<GLfloat, n, 1> & vector, int numVecs) {
        //nsgl::sendUniform(*this, name, vector, numVecs);
		typedef void(*UniformFuncType) (GLint, GLsizei, const GLfloat *);
		static UniformFuncType uniFuncs[4] = {
			(UniformFuncType)glUniform1fv,
			(UniformFuncType)glUniform2fv,
			(UniformFuncType)glUniform3fv,
			(UniformFuncType)glUniform4fv
		};

		if (prog.getUniform(uniName).isValid() && n <= 4) {
			(uniFuncs[n - 1])(prog.getUniform(uniName).location, numVecs,
				vec.data());
		}
    }*/
    
    template <typename T>
    void Program::attach(const std::string & name, TextureBufferObject<T> & tbo) {
        tbo.glBind();
        glSendUniform(name, tbo.whichTextureUnit);
    }
}

#endif // ____nsgl_Program__
