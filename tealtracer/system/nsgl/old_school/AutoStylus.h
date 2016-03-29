///
/// AutoStylus.h
/// ------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_AutoStylus__
#define ____nsgl_AutoStylus__

#include "BasicStylus.h"

namespace nsgl {
   
#ifdef MODERN_OPENGL
#if OPENGL_MAJOR_VERSION >= 4
/// OpenGL 4.0 Shaders
    #define AUTO_STYLUS_VERTEX_SHADER \
     "#version 400\n\nin vec3 obj_position;\nin vec4 obj_color;\n\nuniform mat4 MV;\nuniform mat4 P;\n\nout vec4 cam_color;\n\nvoid main() {\nvec4 cam_position4 = MV * vec4(obj_position, 1.0);\ncam_color = obj_color;\ngl_Position = P * cam_position4;\n}\n"
    #define AUTO_STYLUS_FRAGMENT_SHADER \
     "#version 400\n\nout vec4 frag_color;\n\n\nin vec4 cam_color;\n\nvoid main() {\nfrag_color = cam_color;\n}\n"
#else
/// Opengl 3.2 Shaders
    #define AUTO_STYLUS_VERTEX_SHADER \
     "#version 150\n\nin vec3 obj_position;\nin vec4 obj_color;\n\nuniform mat4 MV;\nuniform mat4 P;\n\nout vec4 cam_color;\n\nvoid main() {\nvec4 cam_position4 = MV * vec4(obj_position, 1.0);\ncam_color = obj_color;\ngl_Position = P * cam_position4;\n}\n"
    #define AUTO_STYLUS_FRAGMENT_SHADER \
     "#version 150\n\nout vec4 frag_color;\n\n\nin vec4 cam_color;\n\nvoid main() {\nfrag_color = cam_color;\n}\n"
#endif
#else
/// OpenGL 2.0 Shaders
    #define AUTO_STYLUS_VERTEX_SHADER \
     "#version 110\n\nattribute vec3 obj_position;\nattribute vec4 obj_color;\n\nuniform mat4 MV;\nuniform mat4 P;\n\nvarying vec4 cam_color;\n\nvoid main() {\nvec4 cam_position4 = MV * vec4(obj_position, 1.0);\ncam_color = obj_color;\ngl_Position = P * cam_position4;\n}\n"
    #define AUTO_STYLUS_FRAGMENT_SHADER \
     "#version 110\n\nvec4 frag_color;\n\n\nvarying vec4 cam_color;\n\nvoid main() {\nfrag_color = cam_color;\ngl_FragColor = frag_color;\n}\n"
#endif
    
    /// Represents a close approximation of immediate-mode rendering in opengl.
    /// It comes with its own shaders and only requires a set of matrix stacks
    /// that it sends when rendering objects. So use it like `nsgl::BasicStylus`
    /// except you only need to add a call to `nsgl::AutoStylus.setMatrices(...)`
    /// for setting the projection and model-view matrix.
    class AutoStylus : public BasicStylus {
    public:
        
        nsgl::Mat4f P, MV;
        
        /// The global program shared by each AutoStylus.
        static Program autoStylusProg;
        /// Indicates whether the program has been setup.
        static bool progIsSetup;
        
        /// Sets up the global auto-stylus program.
        static void setupProgram() {
            if (!progIsSetup) {
                autoStylusProg.vertexShader
                 = Shader::vertexShader("auto_stylus_vert",
                 AUTO_STYLUS_VERTEX_SHADER);
                autoStylusProg.fragmentShader
                 = Shader::fragmentShader("auto_stylus_frag",
                 AUTO_STYLUS_FRAGMENT_SHADER);
                autoStylusProg.ignoreWarnings = true;
                progIsSetup = true;
            }
        }
        
        /// Creates a stylus using the globally shared auto-stylus program.
        AutoStylus() : BasicStylus(&autoStylusProg) {
            P = MV = nsgl::Mat4f::Identity();
            setupProgram();
        }
        
        /// Set the model-view and projection matrices to be sent before each
        /// draw call.
        virtual void setMatrices(nsgl::Mat4f MV, nsgl::Mat4f P) {
            this->MV = MV;
            this->P = P;
        }
        
        /// Draws `model` but sends the model-view and projection matrices.
        virtual void drawData(nsgl::OldSchoolModel & model, bool destroyData = true) {
            program->glBind();
            
            program->glSendUniform("P", P);
            program->glSendUniform("MV", MV);
            
            BasicStylus::drawData(model, destroyData);
            
            program->glUnbind();
        }
    };
    
}


#endif // ____nsgl_AutoStylus__



