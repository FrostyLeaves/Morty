#include "private_header.hlsl"

[[vk::binding(1,0)]]Texture2D U_HDR_OriginTex;
[[vk::binding(2,0)]]sampler defaultSampler;
[[vk::binding(3,0)]]float U_HDR_AverageLum;

struct VS_OUT_GAUSSIAN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PS_OUT_HDR
{
    float4 color0 : SV_Target0;
    float4 color1 : SV_Target1;
};

PS_OUT_HDR PS(VS_OUT_GAUSSIAN input) : SV_Target
{
    PS_OUT_HDR output;

    float4 f4Color = U_HDR_OriginTex.Sample(defaultSampler, input.uv);
    f4Color.xyz = f4Color.xyz / U_HDR_AverageLum;
    f4Color.xyz = f4Color.xyz / (float3(1.0f, 1.0f, 1.0f) + f4Color.xyz);

    output.color0.rgb = f4Color.xyz;
    output.color0.a = f4Color.a;

    float fBrightness = dot(f4Color.rgb, float3(0.2126, 0.7152, 0.0722));
    float fThreshold = 0.75;

    output.color1.rgb = (fBrightness > fThreshold) ? f4Color.rgb : float3(0.0, 0.0, 0.0);
    output.color1.a = f4Color.a;

    return output;
}