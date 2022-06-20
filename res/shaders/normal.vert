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
uniform mat4 br_WorldToObject;

void main()
{
	gl_Position = br_ObjectToClip * vec4(aPos, 1.0);
	texCoord = aTex;
	
	vec3 normal = normalize(aNormal * mat3(br_WorldToObject));
	vec3 tangent = normalize(mat3(br_ObjectToWorld) * aTangent);
	vec3 bitangent = cross(normal, tangent);

	TBN = mat3(tangent, bitangent, normal);
}