#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aTangent;

// out vec3 normal;

out vec2 texCoord;
out mat3 TBN;

uniform mat4 br_ObjectToClip;
uniform mat4 br_ObjectToWorld;

void main()
{
	gl_Position = br_ObjectToClip * vec4(aPos, 1.0);
	// normal = mat3(br_ObjectToWorld) * aNormal;
	texCoord = aTex;

	vec3 aBitangent = cross(aNormal, aTangent);
	vec3 T = normalize(vec3(br_ObjectToWorld * vec4(aTangent,   0.0)));
	vec3 B = normalize(vec3(br_ObjectToWorld * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(br_ObjectToWorld * vec4(aNormal,    0.0)));
	TBN = mat3(T, B, N);
}