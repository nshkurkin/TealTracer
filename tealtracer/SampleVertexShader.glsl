#version 400
// http://antongerdelan.net/opengl/vertexbuffers.html

in vec3 vertex_position;
in vec3 vertex_color;

out vec3 color;

void main() {
    color = vertex_color;
    gl_Position = vec4(vertex_position, 1.0);
}
