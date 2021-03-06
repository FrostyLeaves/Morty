#include "light_deferred.hlsl"

struct PS_OUT
{
    float4 target0: SV_Target0;
};

float3 GetPixelColor(VS_OUT input, float3 f3Color)
{
    return AdditionAllLights(input, f3Color);
}

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;
    
    float3 f3Color = float3(0.0, 0.0, 0.0);

    output.target0 = float4(GetPixelColor(input, f3Color), 1.0f);
    
    return output;
}