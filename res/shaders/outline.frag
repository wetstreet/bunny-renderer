#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;

uniform vec4 _DRAW_COLOR;
uniform sampler2D tex0;

void main()
{
	FragColor = _DRAW_COLOR;
}