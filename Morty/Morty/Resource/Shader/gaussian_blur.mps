#include "private_header.hlsl"

[[vk::binding(1,0)]]Texture2D U_HDR_OriginTex;
[[vk::binding(2,0)]]sampler defaultSampler;
[[vk::binding(3,0)]]float2 U_HDR_BlurOffset;

struct VS_OUT_HDR
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS(VS_OUT_HDR input) : SV_Target
{

    float4 f4Color = float4(0, 0, 0, 0);
    
    float2 f2PixelSize = U_HDR_BlurOffset;

    f4Color += 0.40 * U_HDR_OriginTex.Sample(defaultSampler, input.uv);
    f4Color += 0.15 * U_HDR_OriginTex.Sample(defaultSampler, input.uv + f2PixelSize);
    f4Color += 0.15 * U_HDR_OriginTex.Sample(defaultSampler, input.uv - f2PixelSize);
    f4Color += 0.10 * U_HDR_OriginTex.Sample(defaultSampler, input.uv + f2PixelSize * 2.0f);
    f4Color += 0.10 * U_HDR_OriginTex.Sample(defaultSampler, input.uv - f2PixelSize * 2.0f);
    f4Color += 0.05 * U_HDR_OriginTex.Sample(defaultSampler, input.uv + f2PixelSize * 6.0f);
    f4Color += 0.05 * U_HDR_OriginTex.Sample(defaultSampler, input.uv - f2PixelSize * 6.0f);

    return f4Color;
}