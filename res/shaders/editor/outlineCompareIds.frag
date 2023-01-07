#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D _MainTex;
uniform vec4 _MainTex_TexelSize;

// 8 tap search around the current pixel to
// see if it borders with an object that has a
// different object id
const vec2 kOffsets[8] = vec2[](
	vec2(-1,-1),
	vec2(0,-1),
	vec2(1,-1),
	vec2(-1,0),
	vec2(1,0),
	vec2(-1,1),
	vec2(0,1),
	vec2(1,1)
);

void main()
{
	vec4 currentTexel = texture(_MainTex, texCoord);
	if (currentTexel.r == 0)
	{
		FragColor = currentTexel;
		return;
	}

	// if the current texel borders with a
	// texel that has a differnt object id
	// set the alpha to 0. This implies an
	// edge.
	for (int tap = 0; tap < 8; ++tap)
	{
		float id = texture(_MainTex, texCoord + (kOffsets[tap] * _MainTex_TexelSize.xy)).r;
		if (id != 0 && id - currentTexel.r != 0)
		{
			currentTexel.a = 0;
		}
	}

	FragColor = currentTexel;
	return;
}