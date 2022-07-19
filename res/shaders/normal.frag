#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int FragColor2;

in vec2 texCoord;
in mat3 TBN;
in vec3 posWorld;
in vec4 posLight;

uniform vec4 _Color;
uniform float _Metallic;
uniform float _Roughness;
uniform float _Cutoff;

uniform vec3 _AmbientColor;
uniform int _ObjectID;
uniform vec3 _WorldSpaceCameraPos;
uniform vec3 _MainLightPosition;
uniform vec3 _MainLightColor;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metalMap;
uniform sampler2D shadowMap;

float calcSoftShadow(vec3 normal)
{
	float shadow = 0.0f;
	vec3 lightCoords = posLight.xyz / posLight.w;
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
	return shadow;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 LightingPhysicallyBased(vec3 N, vec3 V, vec3 albedo, vec3 lightColor, vec3 lightDirectionWS, float lightAttenuation)
{
	vec4 metalMapColor = texture(metalMap, texCoord);
	float metallic = metalMapColor.b * _Metallic;
	float roughness = metalMapColor.g * _Roughness;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

	// calculate radiance
	vec3 L = lightDirectionWS;
	float NdotL = max(dot(N, L), 0.0);
	vec3 radiance = lightColor * lightAttenuation * NdotL;

	// cook-torrance brdf
	vec3 H = normalize(V + lightDirectionWS);
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	vec3 brdf = kD * albedo / PI + max(specular, 0);
	return brdf * radiance;
}

void main()
{
	vec4 albedo = texture(albedoMap, texCoord);
	albedo.rgb *= _Color.rgb;
	
    if(albedo.a < _Cutoff)
        discard;

	vec3 normal = texture(normalMap, texCoord).rgb;
	normal = normal * 2.0 - 1.0;
	normal = normalize(TBN * normal);

	if (!gl_FrontFacing)
		normal = -normal;
	
	float shadow = calcSoftShadow(normal);

    vec3 N = normal;
    vec3 V = normalize(_WorldSpaceCameraPos - posWorld);
	
	vec3 color = LightingPhysicallyBased(normal, V, albedo.rgb, _MainLightColor, _MainLightPosition, 1.0 - shadow);

    color += _AmbientColor.rgb * albedo.rgb;
	
    // color = color / (color + vec3(1.0)); // Reinhard tonemapping
    // color = pow(color, vec3(1.0/2.2)); // gamma correction

	FragColor = vec4(color, albedo.a);
	FragColor2 = _ObjectID;
}