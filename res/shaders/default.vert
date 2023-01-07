#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out vec2 texCoord;
out vec3 normal;
// out vec4 fragPosLight;
// out vec4 worldPos;

uniform mat4 br_ObjectToClip;
uniform mat4 br_ObjectToWorld;
uniform mat4 br_WorldToObject;
// uniform mat4 lightProjection;

void main()
{
	gl_Position = br_ObjectToClip * vec4(aPos, 1.0);
	normal = normalize(aNormal * mat3(br_WorldToObject));
	texCoord = aTex;
	// worldPos = br_ObjectToWorld * vec4(aPos, 1.0);
	// fragPosLight = lightProjection * br_ObjectToWorld * vec4(aPos, 1.0);
}