///
/// OldSchoolModel.h
/// ----------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_OldSchoolModel__
#define ____nsgl_OldSchoolModel__

#include <string>
#include <vector>
#include <Eigen/Dense>

#include <src/nsgl/nsgl_types.h>
#include <src/nsgl/nsgl_ext_functions.h>
#include <src/nsgl/Model.h>

namespace nsgl {
    
    /// Represents a basic model that wraps around how `BasicStylus` would
    /// draw a model.
    class OldSchoolModel : public Model {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        
        /// The main attribute object for drawing this model.
        VertexArrayObject vao;
        /// The positions of this model.
        nsgl::V3fBO positions;
        /// The normals at each vertex.
        nsgl::V3fBO normals;
        /// The colors of each vertex.
        nsgl::V4fBO colors;
        /// The indices of the object.
        nsgl::UIBO indices;
        
        /// The method by which this model is drawn.
        GLenum drawType;
        
        /// Creates a model with the given draw `type`.
        OldSchoolModel(GLenum type = GL_TRIANGLES) : Model() {
            positions = nsgl::V3fBO::arrayBuffer();
            normals   = nsgl::V3fBO::arrayBuffer();
            colors    = nsgl::V4fBO::arrayBuffer();
            indices  = nsgl::UIBO::elementArrayBuffer();
            
            drawType = type;
            vao = VertexArrayObject();
        }
        
        /// Removes all associated GL and vertex data of this model.
        virtual void deleteAndDetach() {
            positions.glFree();
            normals.glFree();
            colors.glFree();
            indices.glFree();
            vao.glFree();
        }
        
        /// Draws this model using `prog` and assumes it has the three variables
        /// `obj_position`, `obj_normal`, and `obj_color`.
        virtual void draw(Program & prog) {
            VertexAttribute pos = prog.getVec3fAttribute("obj_position");
            VertexAttribute nor = prog.getVec3fAttribute("obj_normal");
            VertexAttribute col = prog.getVec4fAttribute("obj_color");
            
            ///
            positions.glSendData(GL_DYNAMIC_DRAW);
            normals.glSendData(GL_DYNAMIC_DRAW);
            colors.glSendData(GL_DYNAMIC_DRAW);
            indices.glSendData(GL_DYNAMIC_DRAW);
            
            ///
            nsgl::attachDataAttributeArray(vao, positions, pos);
            nsgl::attachDataAttributeArray(vao, normals, nor);
            nsgl::attachDataAttributeArray(vao, colors, col);
            
            ///
            vao.glBind();
            indices.glBind();
            glDrawElements(drawType, indices.data.size(), GL_UNSIGNED_INT, 0);
        }
    };
    
}

#endif // ____nsgl_OldSchoolModel__
