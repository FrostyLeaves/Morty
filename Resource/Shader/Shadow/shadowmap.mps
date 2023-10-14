#include "../Internal/internal_uniform_model.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

