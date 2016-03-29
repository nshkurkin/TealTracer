///
/// VertexArrayObject.h
/// -------------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_VertexArrayObject__
#define ____nsgl_VertexArrayObject__

#include <src/nsgl/nsgl_types.h>
//#include "../../nsgl_types.h"

namespace nsgl {
    
    /// Represents an opengl Vertex Array Buffer object. It is used to collect
    /// together various DataBufferObjects together for drawing. Note that this
    /// is required by OpenGL 3.2+. For an example of how to use it,
    /// see `Program`.
    struct VertexArrayObject {
        /// The opengl handle for this vao.
        GLuint handle;
        /// Whether or not we have alocated this vao yet in opengl.
        bool allocated;
        
        /// Creates an empty vao.
        VertexArrayObject() : handle(-1), allocated(false) {}
        
        /// Allocates this vao, if needed.
        void glAlloc() {
            if (!allocated) {
                glGenVertexArrays(GLsizei(1), &handle);
                allocated = isValid();
            }
        }
        
        /// Frees the associated opengl content of thie vao.
        void glFree() {
            if (allocated) {
                glDeleteVertexArrays(GLsizei(1), &handle);
                handle = -1;
                allocated = false;
            }
        }
        
        /// Makes this vao the current vao.
        void glBind() {
            glAlloc();
            glBindVertexArray(GLuint(handle));
        }
        /// Make this vao no longer the current vao.
        void glUnbind() {glBindVertexArray(GLuint(0));}
        /// Returns whether this vao is valid.
        bool isValid() {return handle > 0;}
    };
    
    typedef VertexArrayObject VAO;
}

#endif // ____nsgl_VertexArrayObject__
