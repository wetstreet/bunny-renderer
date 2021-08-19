#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;

uniform vec3 _MainLightPosition;
uniform vec3 _MainLightColor;
uniform sampler2D tex0;

void main()
{
	float nl = max(dot(normal, _MainLightPosition), 0);
	vec4 albedo = texture(tex0, texCoord);
	vec3 color = albedo.rgb * _MainLightColor * nl;
	FragColor = vec4(color, albedo.a);
}