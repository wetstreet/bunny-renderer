#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightProjection;
uniform mat4 br_ObjectToWorld;

void main()
{
    gl_Position = lightProjection * br_ObjectToWorld * vec4(aPos, 1.0);
}