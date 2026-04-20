#version 450
#include "common.glsl"

// Fragment shader outputs colors
layout(location = 0) in vec2 vertex_uv;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

void main() {
    //outColor = vec4(vertex_uv, 0, 1);
    outColor = texture(texture_sampler, vertex_uv);
    //vec4(1.0, 0.0, 0.5, 1.0);
}
