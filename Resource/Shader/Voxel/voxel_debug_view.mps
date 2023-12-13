#include "../Internal/internal_uniform_global.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Internal/internal_mesh.hlsl"
#include "../Voxel/voxel_function.hlsl"

struct VS_OUT
{    
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
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

    float3 f3Normal = input.normal;

    uint nAnisoIdx = GetVoxelSimilarFaceIndex(f3Normal);

    float4 f4VoxelColor = float4(
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 0]),
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 1]),
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 2]),
        VoxelUintToFloat(u_rwVoxelTable[voxelTableIdx].nBaseColor[nAnisoIdx * 4 + 3])
    );
    
    uint nVoxelColorNum = u_rwVoxelTable[voxelTableIdx].nVoxelCount[nAnisoIdx]; 

    if (nVoxelColorNum > 0)
    {
        output.f4Color = f4VoxelColor;
        output.f4Color.a = 1.0f;
    }
    else
    {
        output.f4Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    return output;

}