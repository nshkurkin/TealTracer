///
/// BasicStylus.h
/// -------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_BasicStylus__
#define ____nsgl_BasicStylus__

#include "OldSchoolModel.h"

namespace nsgl {
    
    /// Represents a wrapper around Old-School opengl immediate mode rendering.
    /// In particular, you can use it almost exactly ike immediate mode rendering
    /// except you must provide your own shader. The idea of calling it "Stylus"
    /// was supposed to hint that this class is kind of like a digital stylus
    /// you an use to draw your own procedural shapes. For example:
    ///
    /// <pre>
    /// nsgl::BasicStylus stylus = nsgl::BasicStylus(&myProg);
    /// // Draw Two Triangles
    /// stylus.begin(GL_TRIANGLES);
    /// // Set the color
    /// stylus.color(1.0, 0.0, 0.0);
    /// // Set the normal
    /// stylus.normal(0, 0, 1.0);
    /// // Triangle 1
    /// stylus.vertex(0, 0, 0);
    /// stylus.vertex(1, 0, 0);
    /// stylus.vertex(1, 1, 0);
    /// // Change the color
    /// stylus.color(0.0, 1.0, 0.0);
    /// // Next triangle
    /// stylus.vertex(0, 0, 0);
    /// stylus.vertex(1, 1, 0);
    /// stylus.vertex(0, 1, 0);
    /// // Parse the data and send it immediately to `myProg`
    /// stylus.end();
    /// </pre>
    ///
    class BasicStylus {
    public:
        /// Represents a general vertex that is added onto the `vertices` vector
        /// each time `.vertex()` is called.
        struct Vertex {
            nsgl::Vec3f position, normal;
            nsgl::Vec4f color;
			nsgl::Vec2f texCoord;
            Vertex(nsgl::Vec3f position, nsgl::Vec3f normal, nsgl::Vec4f color)
             : position(position), normal(normal), color(color) {
				 texCoord = nsgl::Vec2f(0, 0);
			 }
        };
        
        /// The program to draw to.
        Program * program;
        /// The current set of vertices in this stylus.
        std::vector< Vertex > vertices;
        
        /// The normal to add to the next vertex.
        nsgl::Vec3f drawNormal;
        /// The color to add to the next vertex.
        nsgl::Vec4f drawColor;
        /// The a parsed shape should be drawn.
        GLenum drawType;
        
        /// Creates a stylus using the given program.
        BasicStylus(Program * program = NULL) : program(program) {
            drawColor = nsgl::Vec4f(0, 0, 0, 1);
            drawNormal = nsgl::Vec3f(0, 0, 0);
            
            drawType = GL_POINTS;
        }
        
        /// Starts a new stylus object. Note that this will clear the current
        /// vertex data.
        virtual void begin(GLenum type) {
            clearData();
            drawType = type;
        }
        
        /// Set the alpha of the current draw color.
        virtual void alpha(float a) {drawColor.w() = a;}
        virtual void color(nsgl::Vec4f col) {drawColor = col;}
        virtual void color(float r, float g, float b, float a) {
            color(nsgl::Vec4f(r, g, b, a));
        }
        virtual void color(nsgl::Vec3f col) {drawColor.segment<3>(0) = col;}
        virtual void color(float r, float g, float b) {color(nsgl::Vec3f(r, g, b));}
        
        
        virtual void normal(nsgl::Vec3f nor) {drawNormal = nor;}
        virtual void normal(float x, float y, float z) {normal(nsgl::Vec3f(x, y, z));}
        
        virtual void vertex(nsgl::Vec4f pos) {vertex(nsgl::Vec3f(pos.segment<3>(0)));}
        virtual void vertex(nsgl::Vec3f pos) {
            vertices.push_back(Vertex(pos, drawNormal, drawColor));
        }
        virtual void vertex(float x, float y, float z) {vertex(nsgl::Vec3f(x, y, z));}
        virtual void vertex(nsgl::Vec2f pos) {
            nsgl::Vec3f pos3(0, 0, 0);
            pos3.segment<2>(0) = pos;
            vertex(pos3);
        }
        virtual void vertex(float x, float y) {vertex(nsgl::Vec2f(x, y));}
        
        /// If there is a program present, the data in the stylus is parsed and
        /// then drawn using `program`. Then the present data is cleared.
        virtual void end() {
            if (program != NULL)
                parseAndDrawData();
            clearData();
        }
        
        /// Removes all of the current verte data. This will not reset the
        /// current draw color and normal.
        virtual void clearData() {vertices.clear();}
        
        /// Creates an nsgl::OldSchoolModel containing all of the vertex data
        /// up to when it is called.
        virtual nsgl::OldSchoolModel parseData() {
            nsgl::OldSchoolModel model(drawType);
            nsgl::V3fBO & positions(model.positions);
            nsgl::V3fBO & normals(model.normals);
            nsgl::V4fBO & colors(model.colors);
            nsgl::UIBO & indices(model.indices);
            
            for (int i = 0; i < (int) vertices.size(); i++) {
                positions.add(vertices[i].position);
                normals.add(vertices[i].normal);
                colors.add(vertices[i].color);
                indices.add(nsgl::UInt(i));
            }
            return model;
        }
        
        /// Draws a given model using the current program. If `destroyData` is
        /// TRUE, then the model will have its data erased upon completion.
        virtual void drawData(nsgl::OldSchoolModel & model, bool destroyData = true) {
            model.draw(*program);
            if (destroyData)
                model.deleteAndDetach();
        }
        
        /// Equivalent to `parseData()` being fed to `drawData(..., true)`.
        virtual void parseAndDrawData() {
            OldSchoolModel model = parseData();
            drawData(model, true);
        }
	};
}

#endif // ____nsgl_BasicStylus__

