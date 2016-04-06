#version 400
// http://antongerdelan.net/opengl/vertexbuffers.html

in vec2 texcoord;
out vec4 frag_color;

uniform sampler2D tex;

void main() {
    frag_color = texture(tex, texcoord).rgba;
}
