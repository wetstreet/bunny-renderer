#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int FragColor2;

in vec2 texCoord;

uniform vec4 _Color;
uniform int _ObjectID;
uniform sampler2D tex0;

void main()
{
	vec4 albedo = texture(tex0, texCoord);
	vec3 color = albedo.rgb * _Color.rgb;
	FragColor = vec4(color, albedo.a);
	FragColor2 = _ObjectID;
}