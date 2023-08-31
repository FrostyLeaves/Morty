#include "voxelizer_struct.hlsl"



void PS_MAIN(VS_OUT input)
{

    int3 voxelPosition = int3((input.pos - f3VoxelOrigin) / fVoxelStep);

	int voxelTableIdx = int(voxelPosition.z * (fResolution * fResolution) +
                    voxelPosition.y * fResolution +
                    voxelPosition.x);
    
    uint temp = 0;
    InterlockedAdd(voxelTable[voxelTableIdx].nVoxelCount, 1, temp);
    
}