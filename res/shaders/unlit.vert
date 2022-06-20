#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out vec2 texCoord;

uniform mat4 br_ObjectToClip;
uniform mat4 br_ObjectToWorld;
uniform mat4 br_WorldToObject;

void main()
{
	gl_Position = br_ObjectToClip * vec4(aPos, 1.0);
	texCoord = aTex;
}