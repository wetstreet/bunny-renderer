#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int FragColor2;

// in vec3 normal;

in mat3 TBN;
in vec2 texCoord;

uniform int _ObjectID;
uniform vec3 _MainLightPosition;
uniform vec3 _MainLightColor;
uniform sampler2D tex0;
uniform sampler2D normalMap;

void main()
{
	vec3 normal = texture(normalMap, texCoord).rgb;
	normal = normal * 2.0 - 1.0;   
	normal = normalize(TBN * normal); 

	float nl = max(dot(normal, _MainLightPosition), 0);

	vec4 albedo = texture(tex0, texCoord);
	vec3 color = albedo.rgb * _MainLightColor * nl;
	FragColor = vec4(color, albedo.a);
	FragColor2 = _ObjectID;
}