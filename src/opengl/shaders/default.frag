#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;

uniform vec3 _MainLightPosition;
uniform sampler2D tex0;

void main()
{
	float diff = max(dot(normal, _MainLightPosition), 0);
	FragColor = texture(tex0, texCoord) * diff;
}