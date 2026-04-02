#version 450
#include "common.glsl"

// Fragment shader outputs colors
layout(location = 0) in vec2 vertex_uv;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vertex_uv, 0, 1);
    //vec4(1.0, 0.0, 0.5, 1.0);
}
