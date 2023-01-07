#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D _MainTex;
uniform vec4 _MainTex_TexelSize;
uniform vec2 _BlurDirection;

// 9-tap Gaussian kernel, that blurs green & blue channels,
// keeps red & alpha intact.
const vec4 kCurveWeights[9] = vec4[](
	vec4(0,0.0204001988,0.0204001988,0),
	vec4(0,0.0577929595,0.0577929595,0),
	vec4(0,0.1215916882,0.1215916882,0),
	vec4(0,0.1899858519,0.1899858519,0),
	vec4(1,0.2204586031,0.2204586031,1),
	vec4(0,0.1899858519,0.1899858519,0),
	vec4(0,0.1215916882,0.1215916882,0),
	vec4(0,0.0577929595,0.0577929595,0),
	vec4(0,0.0204001988,0.0204001988,0)
);

void main()
{
	vec2 uv_step = _MainTex_TexelSize.xy * _BlurDirection;
	vec2 uv = texCoord - uv_step * 4;
	vec4 col = vec4(0);
	for (int tap = 0; tap < 9; ++tap)
	{
		col += texture(_MainTex, uv) * kCurveWeights[tap];
		uv += uv_step;
	}
	FragColor = col;
	return;
}