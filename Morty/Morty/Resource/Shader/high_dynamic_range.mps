#include "private_header.hlsl"

[[vk::binding(1,0)]]Texture2D U_HDR_OriginTex;
[[vk::binding(2,0)]]sampler defaultSampler;

struct VS_OUT_HDR
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS(VS_OUT_HDR input) : SV_Target
{
    float4 f4Color = U_HDR_OriginTex.Sample(defaultSampler, input.uv);

    if (f4Color.r > 1.0f ||f4Color.g > 1.0f ||f4Color.b > 1.0f)
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return f4Color;
}