#include "voxelizer_header.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
	float3 normal : NORMAL;
};

void PS_MAIN(VS_OUT input)
{
    
    int3 voxelPosition = getVoxelMapUVW(input.pos.xyz);

	int voxelTableIdx = int(voxelPosition.z * (voxelMapSetting.fResolution * voxelMapSetting.fResolution) +
                    voxelPosition.y * voxelMapSetting.fResolution +
                    voxelPosition.x);
    
    uint temp = 0;
    InterlockedAdd(rwVoxelTable[voxelTableIdx].nVoxelCount, 1, temp);
    
}