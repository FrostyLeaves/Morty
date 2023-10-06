#include "voxelizer_header.hlsl"

uint FloatToUint(float value)
{
    return uint(value * 100);
}

void PS_MAIN(VS_OUT input)
{
    
    uint3 voxelUVW = WorldPositionToVoxelUVW(voxelMapSetting, input.worldPos.xyz);

	int voxelTableIdx = VoxelUVWToInstanceId(voxelMapSetting, voxelUVW);
    
    float3 color;

    uint temp = 0;
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nVoxelCount, 1, temp);
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor_R, FloatToUint(color.r), temp);
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor_G, FloatToUint(color.g), temp);
    InterlockedAdd(u_rwVoxelTable[voxelTableIdx].nBaseColor_B, FloatToUint(color.b), temp);
    
}