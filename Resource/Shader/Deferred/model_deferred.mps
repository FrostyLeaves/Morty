#include "../Deferred/light_deferred.hlsl"

struct PS_OUT
{
    float4 target0: SV_Target;
};

float3 GetPixelColor(VS_OUT input)
{
    return AdditionAllLights(input);
}

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    
    output.target0 = float4(GetPixelColor(input), 1.0f);
    

    return output;
}