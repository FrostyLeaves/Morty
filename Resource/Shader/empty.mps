#include "inner_constant.hlsl"
#include "inner_functional.hlsl"
#include "model_struct.hlsl"

struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

float4 PS(VS_OUT_EMPTY input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

