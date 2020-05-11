#include "privateHeader.hlsl"

Texture2D testTex;

struct VS_OUT_DP
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS(VS_OUT_DP input) : SV_Target
{
    return testTex.Sample(U_defaultSampler, input.uv);
}

