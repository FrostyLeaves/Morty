#include "../Internal/internal_constant.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Internal/internal_model.hlsl"
#include "../Voxel/voxel_struct_define.hlsl"



PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

    output.f4Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;

}