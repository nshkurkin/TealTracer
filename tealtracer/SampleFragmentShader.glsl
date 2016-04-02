#version 400
// http://antongerdelan.net/opengl/vertexbuffers.html

in vec3 color;
out vec4 frag_color;

void main() {
    frag_color = vec4(color,1.0);
}
