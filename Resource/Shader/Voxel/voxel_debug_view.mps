#include "Internal/internal_constant.hlsl"
#include "Internal/internal_functional.hlsl"
#include "Internal/internal_mesh.hlsl"


struct VS_OUT
{    
    float4 pos : SV_POSITION;
    float4 color : u32COLOR;
};

struct PS_OUT
{
    float4 f4Color: SV_Target;
};

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

    output.f4Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;

}