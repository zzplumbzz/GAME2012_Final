#version 430 core

in vec3 colour;
in vec2 texCoord;
out vec4 frag_colour;

uniform sampler2D texture0;

void main()
{
	vec4 texColor = texture(texture0, texCoord) * vec4(colour, 1.0f);
	if(texColor.a < 0.1)
        discard;
	frag_colour = texColor;
}