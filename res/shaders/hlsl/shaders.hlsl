
cbuffer vsConstants : register(b0)
{
    float4x4 br_ObjectToClip;
    float4x4 br_ObjectToWorld;
    float4x4 br_WorldToObject;
};

cbuffer psConstants : register(b0)
{
    float4 _MainLightPosition;
    float4 _MainLightColor;
};

struct VS_Input {
    float3 pos      : POS;
    float3 normal   : NORMAL;
    float2 uv       : TEX;
    float3 tangent  : TANGENT;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 worldNormal : TEXCOORD1;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(float4(input.pos, 1.0f), br_ObjectToClip);
    output.worldNormal = normalize(mul(br_ObjectToWorld, input.normal).xyz);
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    //return mytexture.Sample(mysampler, input.uv);
    float NdotL = max(0, dot(input.worldNormal, _MainLightPosition));

    float3 finalColor = NdotL;

    return float4(finalColor, 1);
}