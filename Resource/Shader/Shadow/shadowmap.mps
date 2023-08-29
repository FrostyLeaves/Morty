#include "../Internal/internal_model.hlsl"

struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

float4 PS_MAIN(VS_OUT_EMPTY input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

