#version 400
// http://antongerdelan.net/opengl/vertexbuffers.html

in vec3 vertex_position;
in vec2 vertex_texcoord;

out vec2 texcoord;

void main() {
    texcoord = vertex_texcoord;
    gl_Position = vec4(vertex_position, 1.0);
}
