#include "voxelizer_header.hlsl"



void PS_MAIN(VS_OUT input)
{
    
    int3 voxelPosition = getVoxelMapUVW(input.pos);

	int voxelTableIdx = int(voxelPosition.z * (voxelMapSetting.fResolution * voxelMapSetting.fResolution) +
                    voxelPosition.y * voxelMapSetting.fResolution +
                    voxelPosition.x);
    
    uint temp = 0;
    InterlockedAdd(rwVoxelTable[voxelTableIdx].nVoxelCount, 1, temp);
    
}