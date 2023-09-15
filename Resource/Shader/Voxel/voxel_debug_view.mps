#include "Internal/internal_constant.hlsl"
#include "Internal/internal_functional.hlsl"
#include "Internal/internal_model.hlsl"
#include "Voxel/voxelizer_header.hlsl"


struct VS_OUT
{    
    float4 pos : SV_POSITION;
    float4 color : u32COLOR;
};


PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

    output.f4Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;

}