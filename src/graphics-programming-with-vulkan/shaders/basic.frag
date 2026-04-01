#version 450
#include "common.glsl"

// Fragment shader outputs colors
layout(location = 0) in vec4 vertex_color;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vertex_color;
    //vec4(1.0, 0.0, 0.5, 1.0);
}
