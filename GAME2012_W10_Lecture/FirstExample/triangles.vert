#version 430 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;
layout(location = 2) in vec2 vertex_texture;

out vec3 colour;
out vec2 texCoord;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main()
{
	colour = vertex_colour;
	texCoord = vertex_texture;
	gl_Position = MVP * vec4(vertex_position, 1.0f);
}