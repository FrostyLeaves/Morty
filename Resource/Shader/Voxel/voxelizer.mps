#include "voxelizer_header.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
	float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
};

void PS_MAIN(VS_OUT input)
{
    
    uint3 voxelUVW = WorldPositionToVoxelUVW(voxelMapSetting, input.worldPos.xyz);

	int voxelTableIdx = VoxelUVWToInstanceId(voxelMapSetting, voxelUVW);
    
    uint temp = 0;
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nVoxelCount, 1, temp);
    
}