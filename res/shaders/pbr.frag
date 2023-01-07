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

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform samplerCube environmentMap;

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

const float PI = 3.14159265359;

float DistributionGGX(float NdotH, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float NdotH  = max(dot(N, H), 0.0);
	return DistributionGGX(NdotH, roughness);
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

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

vec3 LightingPhysicallyBased(vec3 N, vec3 V, vec3 albedo, vec3 F0, float metallic, float roughness
					, vec3 lightColor, vec3 lightDirectionWS, float lightAttenuation)
{
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

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  

float GeometrySchlickGGX_IBL(float NdotV, float roughness)
{
    float r = roughness;
    float k = (r*r) / 2.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith_IBL(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX_IBL(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX_IBL(NdotL, roughness);
	
    return ggx1 * ggx2;
}
const uint NumSamples = 32;
const float maxLod = 10.0;
float distortion(vec3 Wn)
{
  // Computes the inverse of the solid angle of the (differential) pixel in
  // the cube map pointed at by Wn
  float sinT = sqrt(1.0-Wn.y*Wn.y);
  return sinT;
}
float computeLOD(vec3 Ln, float p)
{
  return max(0.0, (maxLod-1.5) - 0.5 * log2(float(NumSamples) * p * distortion(Ln)));
}
float probabilityGGX(float ndh, float vdh, float Roughness)
{
  return DistributionGGX(ndh, Roughness) * ndh / (4.0*vdh);
}
vec3 SpecularIBL( vec3 SpecularColor , float Roughness, vec3 N, vec3 V )
{
    vec3 SpecularLighting = vec3(0.0);

    for( uint i = 0; i < NumSamples; i++ )
    {
        vec2 Xi = Hammersley( i, NumSamples );
        vec3 H = ImportanceSampleGGX( Xi, N, Roughness );
        vec3 L = 2 * dot( V, H ) * H - V;

        float NoV = max( dot( N, V ), 0.0 );
        float NoL = max( dot( N, L ), 0.0 );
        float NoH = max( dot( N, H ), 0.0 );
        float VoH = max( dot( V, H ), 0.0 );

        if( NoL > 0 )
        {
			float resolution = 512.0; // resolution of source cubemap (per face)
			float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float D   = DistributionGGX(NoH, Roughness);
            float pdf = (D * NoH / (4.0 * VoH)) + 0.0001; 
            float saSample = 1.0 / (float(NumSamples) * pdf + 0.0001);
            float lodS = Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
			
    		// float lodS = Roughness < 0.01 ? 0.0 : computeLOD(L, probabilityGGX(NoH, VoH, Roughness));
            vec3 SampleColor = textureLod( environmentMap , L, lodS).rgb;

            float G = GeometrySmith_IBL( N, V, L, Roughness);
            float Fc = pow( 1 - VoH, 5 );
            vec3 F = (1 - Fc) * SpecularColor + Fc;

            // Incident light = SampleColor * NoL
            // Microfacet specular = D*G*F / (4*NoL*NoV)
            // pdf = D * NoH / (4 * VoH)
            SpecularLighting += SampleColor * F * G * VoH / (NoH * NoV);
        }
    }
    return SpecularLighting / NumSamples;
}

void main()
{
	vec4 albedo = texture(albedoMap, texCoord);
	albedo.rgb = pow(albedo.rgb, vec3(2.2)); // srgb to linear
	albedo.rgb *= pow(_Color.rgb, vec3(2.2)); // srgb to linear
	
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
    vec3 R = reflect(-V, N); 

	vec4 metalMapColor = texture(metalMap, texCoord);
	float metallic = metalMapColor.b * _Metallic;
	float roughness = metalMapColor.g * _Roughness;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

	vec3 color = LightingPhysicallyBased(normal, V, albedo.rgb, F0, metallic, roughness, _MainLightColor, _MainLightPosition, 1.0 - shadow);

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = (1.0 - F) * (1.0 - F0);
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = kD * irradiance * albedo.rgb;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	// vec3 specular = SpecularIBL(F0, roughness, normal, V);

    vec3 ambient = diffuse + specular ;
	
    color += ambient;


    // color = color / (color + vec3(1.0)); // Reinhard tonemapping
    color = pow(color, vec3(1.0/2.2)); // gamma correction

	FragColor = vec4(color, albedo.a);
	FragColor2 = _ObjectID;
}