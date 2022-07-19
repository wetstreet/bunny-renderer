#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D _MainTex;
uniform vec4 _MainTex_TexelSize;
uniform vec4 _OutlineColor;
uniform float _OutlineFade;

void main()
{
	vec4 col = texture(_MainTex, texCoord);

	bool isSelected = col.a > 0.9;
	float alpha = clamp(col.b * 10, 0, 1);
	if (isSelected)
	{
		// outline color alpha controls how much tint the whole object gets
		alpha = _OutlineColor.a;
		vec2 tempLeft = texCoord - _MainTex_TexelSize.xy*2;
		vec2 tempRight = texCoord + _MainTex_TexelSize.xy*2;
		if (tempLeft.x < 0 || tempLeft.y < 0 || tempRight.x > 1 || tempRight.y > 1)
			alpha = 1;
	}
	bool inFront = col.g > 0.0;
	if (!inFront)
	{
		alpha *= 0.3;
		if (isSelected) // no tinting at all for occluded selection
			alpha = 0;
	}
	alpha *= _OutlineFade;
	vec4 outlineColor = vec4(_OutlineColor.rgb, alpha);
	FragColor = outlineColor;
	return;
}