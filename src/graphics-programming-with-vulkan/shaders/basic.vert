#version 450

// Vertex shader outputs positions
//vec2 hardcoded_positions[3] = vec2[](
//	vec2(0.0, -0.5),
//	vec2(0.5, 0.5),
//	vec2(-0.5, 0.5)
//);
//
//vec4 hardcoded_colors[3] = vec4[](
//	vec4(1.0, 0.0, 0.0, 1.0),
//	vec4(0.0, 1.0, 0.0, 1.0),
//	vec4(0.0, 0.0, 1.0, 1.0)
//);

layout(location = 0) in vec3 input_position;
layout(location = 1) in vec3 input_color;

layout(location = 0) out vec4 vertex_color;

layout(push_constant) uniform Model {
	mat4 transformation;
} model;

void main() {
	gl_Position = model.transformation * vec4(input_position, 1.0);
	vertex_color = vec4(input_color, 1.0);
}