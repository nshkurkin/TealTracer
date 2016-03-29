///
/// nsgl_ext_functions.h
/// --------------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_ext_functions__
#define ____nsgl_ext_functions__

#include <src/nsgl/nsgl_types.h>
#include <src/nsgl/gl_interface/gl_interface.h>

namespace nsgl {
    /// Associates `vao` with `attr` and `buffer`. This is needed to ensure that
    /// that `vao` points to `buffer` so that when the contents of `vao` are
    /// drawn, the contents of `buffer` will be sent to `attr`, which is
    /// associated with some program that is currently bound.
    template<typename T>
    void attachDataAttributeArray(VertexArrayObject & vao,
     DataBufferObject<T> & buffer, VertexAttribute & attr) {
        if (attr.isValid() && buffer.data.size() > 0) {
            vao.glBind();
            
            attr.glEnable();
            
            buffer.glBind();
            attr.glSetLayout();
            
            buffer.glUnbind();
            vao.glUnbind();
        }
    }
}

#endif // ____nsgl_ext_functions__


