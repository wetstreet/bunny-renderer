#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int FragColor2;

in mat3 TBN;
in vec2 texCoord;
in vec4 fragPosLight;

uniform vec4 _Color;
uniform vec3 _AmbientColor;
uniform int _ObjectID;
uniform vec3 _MainLightPosition;
uniform vec3 _MainLightColor;
uniform sampler2D tex0;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

void main()
{
	vec3 normal = texture(normalMap, texCoord).rgb;
	normal = normal * 2.0 - 1.0;   
	normal = normalize(TBN * normal); 

	float nl = max(dot(normal, _MainLightPosition), 0);

	vec4 albedo = texture(tex0, texCoord);
	
	float shadow = 0.0f;
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	if (lightCoords.z <= 1.0f)
	{
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;

		float bias = max(0.025f * (1.0f - dot(normal, _MainLightPosition)), 0.0005f);

		int sampleRadius = 2;
		vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
		for (int y = -sampleRadius; y <= sampleRadius; y++)
		{
			for (int x = -sampleRadius; x <= sampleRadius; x++)
			{
				float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;
			}
		}

		shadow /= pow((sampleRadius * 2 + 1), 2);
	}

	vec3 color = albedo.rgb * _Color.rgb * (_MainLightColor * nl * (1.0f - shadow) + _AmbientColor.rgb);
	FragColor = vec4(color, albedo.a);
	FragColor2 = _ObjectID;
}