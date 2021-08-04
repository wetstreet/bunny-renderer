#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 color;

uniform float scale;

void main()
{
	gl_Position = vec4(aPos.x * (scale + 1), aPos.y * (scale + 1), aPos.z * (scale + 1), 1.0);
	color = aColor;
}