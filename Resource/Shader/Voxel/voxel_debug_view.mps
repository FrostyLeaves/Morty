#include "../Internal/internal_uniform_global.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Internal/internal_mesh.hlsl"
#include "../Voxel/voxel_function.hlsl"

struct VS_OUT
{    
    float4 pos : SV_POSITION;
    float4 color : u32COLOR;
    uint voxelIdx : VOXEL_IDX;
};

struct PS_OUT
{
    float4 f4Color: SV_Target;
};

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

	int voxelTableIdx = input.voxelIdx;

    uint nVoxelColorNum = u_rwVoxelTable[voxelTableIdx].nVoxelCount; 

    float4 f4VoxelColor = float4(
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor_R),
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor_G),
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor_B),
        1.0f);
    
    f4VoxelColor.rgb = f4VoxelColor.rgb / nVoxelColorNum;

    output.f4Color = f4VoxelColor;

    return output;

}