///
/// VertexAttribute.h
/// -----------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_VertexAttribute__
#define ____nsgl_VertexAttribute__

#include <string>

#include <src/nsgl/gl_interface/gl.h>
#include <src/nsgl/nsgl_base.h>
//#include "../gl.h"
//#include "../../nsgl_base.h"

namespace nsgl {
    
    /// Represents a connection to a shader program's variable. Use this to set
    /// attribute layouts and to enable a particular attribute. "Attributes" are
    /// the per-vertex data sent to shaders like "positions", "normals", etc.
    /// Usually you a get an attribute directly from a program itself so there
    /// is no need to create one manually. See Program.get*Attribute(...)
    /// methods.
    struct VertexAttribute {
    public:
        /// The name of this attribute in the shader.
        std::string name;
        /// The handle to the shader's attribute location.
        GLint location;
        /// The number of elements this attribute has (e.g. 3 for a vec3).
        GLint numElements;
        /// The type of this attribute. e.g. GL_FLOAT.
        GLenum type;
        
        /// Creates an empty attribute.
        VertexAttribute();
        /// Creates a named sttribute.
        VertexAttribute(std::string name);
        /// Create an attribute linked to a shader.
        VertexAttribute(std::string name, GLint attribLocation, GLint numElements,
         GLenum type);
        
        /// Initializes this attribute in-place.
        void init(std::string name, GLint attribLocation, GLint numElements,
         GLenum type);
        
        /// Enables the attribute given at `location`.
        void glEnable();
        /// Disable the attribute given at `location`.
        void glDisable();
        /// Sets the pointer layout of `location`. For more advanced layout
        /// settings you should do it manually using opengl.
        void glSetLayout();
        
        /// Returns whether `location` is invalid.
        bool isValid();
    };
    
}

#endif // ____nsgl_VertexAttribute__
