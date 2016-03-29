///
///
///
///
///
///

#ifndef ____nsgl_Model__
#define ____nsgl_Model__

#include <src/nsgl/nsgl_base.h>
#include <src/nsgl/gl_interface/shader/Program.h>

namespace nsgl {

    /// Represents an interface that defines what a "model" is. That is, that
    /// it can be drawn. Use this if you have a bunch of models that only need
    /// to be drawn.
    class Model {
    public:
        Model() {}
        virtual ~Model() {}
        
        /// Draws this model using `prog`. This model may make assumptions about
        /// the shader variables available.
        virtual void draw(Program & prog) {}
    };
}

#endif

